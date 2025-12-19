#ifndef POORIAYOUSEFI_API_SERVER_HPP
#define POORIAYOUSEFI_API_SERVER_HPP

#include "coordinator.hpp"
#include "httplib.h"
#include <string>
#include <memory>
#include <chrono>
#include <mutex>
#include <map>

namespace pooriayousefi
{

// ============================================================================
// Request/Response Logging
// ============================================================================

struct RequestLog
{
    std::string request_id;
    std::string endpoint;
    std::string method;
    std::string user_id;
    std::chrono::system_clock::time_point timestamp;
    std::chrono::milliseconds duration;
    int status_code;
    size_t request_size;
    size_t response_size;
};

// ============================================================================
// Performance Metrics
// ============================================================================

struct PerformanceMetrics
{
    size_t total_requests{0};
    size_t successful_requests{0};
    size_t failed_requests{0};
    std::chrono::milliseconds total_duration{0};
    std::chrono::milliseconds min_duration{std::chrono::milliseconds::max()};
    std::chrono::milliseconds max_duration{0};
    
    double average_duration() const
    {
        if (total_requests == 0) return 0.0;
        return static_cast<double>(total_duration.count()) / total_requests;
    }
};

// ============================================================================
// API Server Configuration
// ============================================================================

struct ApiServerConfig
{
    std::string host{"0.0.0.0"};
    int port{8000};
    std::string llm_server_url{"localhost:8080"};
    bool enable_logging{true};
    bool enable_metrics{true};
    RoutingStrategy default_routing{RoutingStrategy::IntentBased};
};

// ============================================================================
// API Server
// ============================================================================

class ApiServer
{
private:
    std::unique_ptr<httplib::Server> server_;
    std::unique_ptr<Coordinator> coordinator_;
    ApiServerConfig config_;
    
    // Logging and metrics
    std::vector<RequestLog> request_logs_;
    std::map<std::string, PerformanceMetrics> endpoint_metrics_;
    mutable std::mutex logs_mutex_;
    mutable std::mutex metrics_mutex_;
    
    // Helper: Generate request ID
    std::string generate_request_id()
    {
        auto now = std::chrono::system_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()
        ).count();
        return "req_" + std::to_string(ms);
    }
    
    // Helper: Log request
    void log_request(
        const std::string& request_id,
        const std::string& endpoint,
        const std::string& method,
        const std::string& user_id,
        std::chrono::milliseconds duration,
        int status_code,
        size_t request_size,
        size_t response_size
    )
    {
        if (!config_.enable_logging) return;
        
        std::lock_guard<std::mutex> lock(logs_mutex_);
        request_logs_.push_back({
            request_id,
            endpoint,
            method,
            user_id,
            std::chrono::system_clock::now(),
            duration,
            status_code,
            request_size,
            response_size
        });
        
        // Keep only last 1000 logs
        if (request_logs_.size() > 1000)
        {
            request_logs_.erase(request_logs_.begin());
        }
    }
    
    // Helper: Update metrics
    void update_metrics(
        const std::string& endpoint,
        std::chrono::milliseconds duration,
        bool success
    )
    {
        if (!config_.enable_metrics) return;
        
        std::lock_guard<std::mutex> lock(metrics_mutex_);
        auto& metrics = endpoint_metrics_[endpoint];
        
        metrics.total_requests++;
        if (success) metrics.successful_requests++;
        else metrics.failed_requests++;
        
        metrics.total_duration += duration;
        if (duration < metrics.min_duration) metrics.min_duration = duration;
        if (duration > metrics.max_duration) metrics.max_duration = duration;
    }
    
    // Helper: Parse JSON body
    std::optional<nlohmann::json> parse_json_body(const httplib::Request& req)
    {
        try
        {
            return nlohmann::json::parse(req.body);
        }
        catch (const std::exception&)
        {
            return std::nullopt;
        }
    }
    
    // Helper: Create error response
    nlohmann::json error_response(const std::string& error, const std::string& details = "")
    {
        nlohmann::json response;
        response["error"] = error;
        if (!details.empty()) response["details"] = details;
        return response;
    }
    
    // Helper: Create success response
    nlohmann::json success_response(const nlohmann::json& data)
    {
        nlohmann::json response;
        response["success"] = true;
        response["data"] = data;
        return response;
    }

public:
    ApiServer(const ApiServerConfig& config = {})
        : config_(config)
    {
        server_ = std::make_unique<httplib::Server>();
        coordinator_ = std::make_unique<Coordinator>(
            config_.llm_server_url,
            config_.default_routing,
            true  // Register default agents
        );
        
        setup_routes();
    }
    
    void setup_routes()
    {
        // Health check
        server_->Get("/health", [](const httplib::Request&, httplib::Response& res)
        {
            nlohmann::json response;
            response["status"] = "ok";
            response["service"] = "intellistant-api";
            res.set_content(response.dump(), "application/json");
        });
        
        // List agents
        server_->Get("/api/agents", [this](const httplib::Request& req, httplib::Response& res)
        {
            auto start = std::chrono::steady_clock::now();
            std::string request_id = generate_request_id();
            
            try
            {
                auto agents = coordinator_->list_agents();
                
                nlohmann::json response;
                response["agents"] = agents;
                response["count"] = agents.size();
                
                auto end = std::chrono::steady_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
                
                log_request(request_id, "/api/agents", "GET", "", duration, 200,
                           req.body.size(), response.dump().size());
                update_metrics("/api/agents", duration, true);
                
                res.set_content(success_response(response).dump(), "application/json");
            }
            catch (const std::exception& e)
            {
                auto end = std::chrono::steady_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
                
                log_request(request_id, "/api/agents", "GET", "", duration, 500,
                           req.body.size(), 0);
                update_metrics("/api/agents", duration, false);
                
                res.status = 500;
                res.set_content(error_response("Internal error", e.what()).dump(),
                               "application/json");
            }
        });
        
        // Create session
        server_->Post("/api/sessions", [this](const httplib::Request& req, httplib::Response& res)
        {
            auto start = std::chrono::steady_clock::now();
            std::string request_id = generate_request_id();
            
            auto json_opt = parse_json_body(req);
            if (!json_opt)
            {
                res.status = 400;
                res.set_content(error_response("Invalid JSON").dump(), "application/json");
                return;
            }
            
            auto json = *json_opt;
            if (!json.contains("user_id"))
            {
                res.status = 400;
                res.set_content(error_response("Missing user_id").dump(), "application/json");
                return;
            }
            
            try
            {
                std::string user_id = json["user_id"];
                std::string session_id = generate_request_id();
                
                coordinator_->create_session(session_id, user_id);
                
                nlohmann::json response;
                response["session_id"] = session_id;
                response["user_id"] = user_id;
                
                auto end = std::chrono::steady_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
                
                log_request(request_id, "/api/sessions", "POST", user_id, duration, 201,
                           req.body.size(), response.dump().size());
                update_metrics("/api/sessions", duration, true);
                
                res.status = 201;
                res.set_content(success_response(response).dump(), "application/json");
            }
            catch (const std::exception& e)
            {
                auto end = std::chrono::steady_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
                
                log_request(request_id, "/api/sessions", "POST", "", duration, 500,
                           req.body.size(), 0);
                update_metrics("/api/sessions", duration, false);
                
                res.status = 500;
                res.set_content(error_response("Internal error", e.what()).dump(),
                               "application/json");
            }
        });
        
        // Send message (main endpoint)
        server_->Post("/api/chat", [this](const httplib::Request& req, httplib::Response& res)
        {
            auto start = std::chrono::steady_clock::now();
            std::string request_id = generate_request_id();
            
            auto json_opt = parse_json_body(req);
            if (!json_opt)
            {
                res.status = 400;
                res.set_content(error_response("Invalid JSON").dump(), "application/json");
                return;
            }
            
            auto json = *json_opt;
            if (!json.contains("message"))
            {
                res.status = 400;
                res.set_content(error_response("Missing message field").dump(),
                               "application/json");
                return;
            }
            
            try
            {
                UserRequest user_req;
                user_req.message = json["message"];
                user_req.user_id = json.value("user_id", "anonymous");
                user_req.session_id = json.value("session_id", "");
                
                if (json.contains("preferred_agent"))
                {
                    user_req.preferred_agent = json["preferred_agent"];
                }
                
                if (json.contains("metadata") && json["metadata"].is_object())
                {
                    for (auto& [key, value] : json["metadata"].items())
                    {
                        if (value.is_string())
                        {
                            user_req.metadata[key] = value;
                        }
                    }
                }
                
                auto result = coordinator_->handle_request(user_req);
                
                auto end = std::chrono::steady_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
                
                if (result)
                {
                    nlohmann::json response;
                    response["agent"] = result->agent_name;
                    response["response"] = result->response;
                    response["tool_results"] = result->tool_results;
                    response["requires_followup"] = result->requires_followup;
                    if (result->next_agent_suggestion)
                    {
                        response["next_agent"] = *result->next_agent_suggestion;
                    }
                    response["agents_used"] = result->agents_used;
                    
                    log_request(request_id, "/api/chat", "POST", user_req.user_id,
                               duration, 200, req.body.size(), response.dump().size());
                    update_metrics("/api/chat", duration, true);
                    
                    res.set_content(success_response(response).dump(), "application/json");
                }
                else
                {
                    log_request(request_id, "/api/chat", "POST", user_req.user_id,
                               duration, 500, req.body.size(), 0);
                    update_metrics("/api/chat", duration, false);
                    
                    res.status = 500;
                    res.set_content(error_response("Request failed", result.error()).dump(),
                                   "application/json");
                }
            }
            catch (const std::exception& e)
            {
                auto end = std::chrono::steady_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
                
                log_request(request_id, "/api/chat", "POST", "", duration, 500,
                           req.body.size(), 0);
                update_metrics("/api/chat", duration, false);
                
                res.status = 500;
                res.set_content(error_response("Internal error", e.what()).dump(),
                               "application/json");
            }
        });
        
        // Multi-agent collaboration
        server_->Post("/api/collaborate", [this](const httplib::Request& req, httplib::Response& res)
        {
            auto start = std::chrono::steady_clock::now();
            std::string request_id = generate_request_id();
            
            auto json_opt = parse_json_body(req);
            if (!json_opt)
            {
                res.status = 400;
                res.set_content(error_response("Invalid JSON").dump(), "application/json");
                return;
            }
            
            auto json = *json_opt;
            if (!json.contains("task") || !json.contains("agents"))
            {
                res.status = 400;
                res.set_content(error_response("Missing task or agents field").dump(),
                               "application/json");
                return;
            }
            
            try
            {
                std::string task = json["task"];
                std::vector<std::string> agents = json["agents"];
                
                auto result = coordinator_->collaborate(task, agents);
                
                auto end = std::chrono::steady_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
                
                if (result)
                {
                    nlohmann::json response;
                    response["agent"] = result->agent_name;
                    response["response"] = result->response;
                    response["tool_results"] = result->tool_results;
                    response["agents_used"] = result->agents_used;
                    
                    log_request(request_id, "/api/collaborate", "POST", "", duration, 200,
                               req.body.size(), response.dump().size());
                    update_metrics("/api/collaborate", duration, true);
                    
                    res.set_content(success_response(response).dump(), "application/json");
                }
                else
                {
                    log_request(request_id, "/api/collaborate", "POST", "", duration, 500,
                               req.body.size(), 0);
                    update_metrics("/api/collaborate", duration, false);
                    
                    res.status = 500;
                    res.set_content(error_response("Collaboration failed", result.error()).dump(),
                                   "application/json");
                }
            }
            catch (const std::exception& e)
            {
                auto end = std::chrono::steady_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
                
                log_request(request_id, "/api/collaborate", "POST", "", duration, 500,
                           req.body.size(), 0);
                update_metrics("/api/collaborate", duration, false);
                
                res.status = 500;
                res.set_content(error_response("Internal error", e.what()).dump(),
                               "application/json");
            }
        });
        
        // Get metrics
        server_->Get("/api/metrics", [this](const httplib::Request& req, httplib::Response& res)
        {
            std::lock_guard<std::mutex> lock(metrics_mutex_);
            
            nlohmann::json response;
            for (const auto& [endpoint, metrics] : endpoint_metrics_)
            {
                nlohmann::json endpoint_data;
                endpoint_data["total_requests"] = metrics.total_requests;
                endpoint_data["successful_requests"] = metrics.successful_requests;
                endpoint_data["failed_requests"] = metrics.failed_requests;
                endpoint_data["average_duration_ms"] = metrics.average_duration();
                endpoint_data["min_duration_ms"] = metrics.min_duration.count();
                endpoint_data["max_duration_ms"] = metrics.max_duration.count();
                
                response[endpoint] = endpoint_data;
            }
            
            res.set_content(success_response(response).dump(), "application/json");
        });
        
        // Get logs
        server_->Get("/api/logs", [this](const httplib::Request& req, httplib::Response& res)
        {
            std::lock_guard<std::mutex> lock(logs_mutex_);
            
            int limit = 100;
            if (req.has_param("limit"))
            {
                limit = std::stoi(req.get_param_value("limit"));
            }
            
            nlohmann::json response = nlohmann::json::array();
            
            int start_idx = std::max(0, static_cast<int>(request_logs_.size()) - limit);
            for (int i = start_idx; i < static_cast<int>(request_logs_.size()); i++)
            {
                const auto& log = request_logs_[i];
                nlohmann::json log_entry;
                log_entry["request_id"] = log.request_id;
                log_entry["endpoint"] = log.endpoint;
                log_entry["method"] = log.method;
                log_entry["user_id"] = log.user_id;
                log_entry["duration_ms"] = log.duration.count();
                log_entry["status_code"] = log.status_code;
                log_entry["request_size"] = log.request_size;
                log_entry["response_size"] = log.response_size;
                
                response.push_back(log_entry);
            }
            
            res.set_content(success_response(response).dump(), "application/json");
        });
        
        // Agent statistics
        server_->Get("/api/stats", [this](const httplib::Request&, httplib::Response& res)
        {
            auto stats = coordinator_->get_agent_usage_stats();
            auto active_sessions = coordinator_->get_active_sessions_count();
            
            nlohmann::json response;
            response["agent_usage"] = stats;
            response["active_sessions"] = active_sessions;
            
            res.set_content(success_response(response).dump(), "application/json");
        });
    }
    
    // Start server
    bool start()
    {
        std::cout << "Starting API server on " << config_.host << ":" << config_.port << std::endl;
        std::cout << "API endpoints:" << std::endl;
        std::cout << "  GET  /health           - Health check" << std::endl;
        std::cout << "  GET  /api/agents       - List agents" << std::endl;
        std::cout << "  POST /api/sessions     - Create session" << std::endl;
        std::cout << "  POST /api/chat         - Send message" << std::endl;
        std::cout << "  POST /api/collaborate  - Multi-agent collaboration" << std::endl;
        std::cout << "  GET  /api/metrics      - Performance metrics" << std::endl;
        std::cout << "  GET  /api/logs         - Request logs" << std::endl;
        std::cout << "  GET  /api/stats        - Agent statistics" << std::endl;
        
        return server_->listen(config_.host, config_.port);
    }
    
    // Stop server
    void stop()
    {
        server_->stop();
    }
};

} // namespace pooriayousefi

#endif // POORIAYOUSEFI_API_SERVER_HPP
