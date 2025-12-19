#ifndef POORIAYOUSEFI_COORDINATOR_HPP
#define POORIAYOUSEFI_COORDINATOR_HPP

#include "agent.hpp"
#include "agents.hpp"
#include <memory>
#include <map>
#include <vector>
#include <string>
#include <optional>
#include <chrono>
#include <expected>

namespace pooriayousefi
{

// ============================================================================
// Coordinator Types
// ============================================================================

struct UserRequest
{
    std::string user_id;
    std::string session_id;
    std::string message;
    std::optional<std::string> preferred_agent;
    nlohmann::json metadata;
    
    UserRequest()
        : metadata(nlohmann::json::object())
    {
    }
};

struct CoordinatorResponse
{
    std::string agent_name;
    std::string response;
    std::vector<std::string> tool_results;
    bool requires_followup;
    std::optional<std::string> next_agent_suggestion;
    int agents_used;
    
    CoordinatorResponse()
        : requires_followup(false), agents_used(1)
    {
    }
};

struct Session
{
    std::string session_id;
    std::string user_id;
    std::vector<UserRequest> request_history;
    std::map<std::string, std::string> context;
    std::chrono::system_clock::time_point created_at;
    std::chrono::system_clock::time_point last_activity;
    
    Session()
        : created_at(std::chrono::system_clock::now())
        , last_activity(std::chrono::system_clock::now())
    {
    }
};

enum class RoutingStrategy
{
    IntentBased,      // Use LLM to analyze intent and route
    KeywordBased,     // Match keywords to agent specializations
    PreferredAgent,   // Use user-specified agent
    RoundRobin        // Distribute load evenly
};

// ============================================================================
// Coordinator Class
// ============================================================================

class Coordinator
{
public:
    explicit Coordinator(
        const std::string& llm_endpoint,
        RoutingStrategy strategy = RoutingStrategy::IntentBased,
        bool verbose = false
    )
        : llm_endpoint_(llm_endpoint)
        , routing_strategy_(strategy)
        , verbose_(verbose)
        , round_robin_index_(0)
    {
        llm_client_ = std::make_shared<LlmClient>(
            llm_endpoint.substr(0, llm_endpoint.find(':')),
            std::stoi(llm_endpoint.substr(llm_endpoint.find(':') + 1))
        );
        
        // Create MCP server for all agents
        mcp_server_ = std::make_shared<McpServer>("Coordinator", "1.0.0");
        ToolRegistry::register_system_tools(*mcp_server_);
        
        // Register default specialized agents
        register_default_agents();
    }
    
    // ========================================================================
    // Agent Management
    // ========================================================================
    
    void register_agent(const std::string& name, std::shared_ptr<Agent> agent)
    {
        if (verbose_)
        {
            std::cout << "[Coordinator] Registering agent: " << name << std::endl;
        }
        
        agents_[name] = agent;
        agent_usage_[name] = 0;
    }
    
    void remove_agent(const std::string& name)
    {
        agents_.erase(name);
        agent_usage_.erase(name);
        
        if (verbose_)
        {
            std::cout << "[Coordinator] Removed agent: " << name << std::endl;
        }
    }
    
    std::vector<std::string> list_agents() const
    {
        std::vector<std::string> names;
        for (const auto& [name, agent] : agents_)
        {
            names.push_back(name);
        }
        return names;
    }
    
    std::shared_ptr<Agent> get_agent(const std::string& name)
    {
        auto it = agents_.find(name);
        if (it != agents_.end())
        {
            return it->second;
        }
        return nullptr;
    }
    
    // ========================================================================
    // Session Management
    // ========================================================================
    
    void create_session(const std::string& session_id, const std::string& user_id)
    {
        Session session;
        session.session_id = session_id;
        session.user_id = user_id;
        sessions_[session_id] = session;
        
        if (verbose_)
        {
            std::cout << "[Coordinator] Created session: " << session_id << std::endl;
        }
    }
    
    void end_session(const std::string& session_id)
    {
        sessions_.erase(session_id);
        
        if (verbose_)
        {
            std::cout << "[Coordinator] Ended session: " << session_id << std::endl;
        }
    }
    
    std::optional<Session> get_session(const std::string& session_id)
    {
        auto it = sessions_.find(session_id);
        if (it != sessions_.end())
        {
            return it->second;
        }
        return std::nullopt;
    }
    
    void update_session_context(const std::string& session_id, const std::string& key, const std::string& value)
    {
        auto it = sessions_.find(session_id);
        if (it != sessions_.end())
        {
            it->second.context[key] = value;
            it->second.last_activity = std::chrono::system_clock::now();
        }
    }
    
    // ========================================================================
    // Request Handling
    // ========================================================================
    
    std::expected<CoordinatorResponse, std::string> handle_request(const UserRequest& request)
    {
        if (verbose_)
        {
            std::cout << "[Coordinator] Handling request: " << request.message << std::endl;
        }
        
        // Store request in session history
        if (!request.session_id.empty())
        {
            auto it = sessions_.find(request.session_id);
            if (it != sessions_.end())
            {
                it->second.request_history.push_back(request);
                it->second.last_activity = std::chrono::system_clock::now();
            }
        }
        
        // Route to appropriate agent
        auto agent_name_result = route_request(request);
        if (!agent_name_result)
        {
            return std::unexpected(agent_name_result.error());
        }
        
        std::string agent_name = *agent_name_result;
        auto agent = get_agent(agent_name);
        if (!agent)
        {
            return std::unexpected("Agent not found: " + agent_name);
        }
        
        if (verbose_)
        {
            std::cout << "[Coordinator] Routed to agent: " << agent_name << std::endl;
        }
        
        // Process request with selected agent
        auto agent_response = agent->process(request.message);
        if (!agent_response)
        {
            return std::unexpected("Agent processing failed: " + agent_response.error());
        }
        
        // Track agent usage
        agent_usage_[agent_name]++;
        
        // Build coordinator response
        CoordinatorResponse response;
        response.agent_name = agent_name;
        response.response = agent_response->content;
        response.tool_results = agent_response->tool_calls_made;
        response.requires_followup = agent_response->stopped_by_limit;
        response.agents_used = 1;
        
        // Suggest next agent if needed
        if (agent_response->stopped_by_limit || should_suggest_followup(request.message))
        {
            response.next_agent_suggestion = suggest_next_agent(agent_name, request.message);
        }
        
        return response;
    }
    
    // ========================================================================
    // Multi-Agent Collaboration
    // ========================================================================
    
    std::expected<CoordinatorResponse, std::string> collaborate(
        const std::string& task,
        const std::vector<std::string>& agent_names
    )
    {
        if (verbose_)
        {
            std::cout << "[Coordinator] Collaborating with " << agent_names.size() << " agents" << std::endl;
        }
        
        std::vector<std::string> all_responses;
        std::vector<std::string> all_tools;
        
        for (const auto& agent_name : agent_names)
        {
            auto agent = get_agent(agent_name);
            if (!agent)
            {
                continue;
            }
            
            if (verbose_)
            {
                std::cout << "[Coordinator] Consulting agent: " << agent_name << std::endl;
            }
            
            auto response = agent->process(task);
            if (response)
            {
                all_responses.push_back(agent_name + ": " + response->content);
                all_tools.insert(all_tools.end(), 
                               response->tool_calls_made.begin(), 
                               response->tool_calls_made.end());
                agent_usage_[agent_name]++;
            }
        }
        
        // Aggregate results
        if (all_responses.empty())
        {
            return std::unexpected("No agents provided valid responses");
        }
        
        std::string aggregated = aggregate_responses(all_responses);
        
        CoordinatorResponse response;
        response.agent_name = "Collaboration";
        response.response = aggregated;
        response.tool_results = all_tools;
        response.requires_followup = false;
        response.agents_used = agent_names.size();
        
        return response;
    }
    
    // ========================================================================
    // Statistics
    // ========================================================================
    
    std::map<std::string, int> get_agent_usage_stats() const
    {
        return agent_usage_;
    }
    
    size_t get_active_sessions_count() const
    {
        return sessions_.size();
    }

private:
    // ========================================================================
    // Routing Implementation
    // ========================================================================
    
    std::expected<std::string, std::string> route_request(const UserRequest& request)
    {
        // Preferred agent takes priority
        if (request.preferred_agent && agents_.count(*request.preferred_agent))
        {
            return *request.preferred_agent;
        }
        
        // Apply routing strategy
        switch (routing_strategy_)
        {
            case RoutingStrategy::IntentBased:
                return route_by_intent(request.message);
            
            case RoutingStrategy::KeywordBased:
                return route_by_keywords(request.message);
            
            case RoutingStrategy::RoundRobin:
                return route_round_robin();
            
            default:
                return route_by_keywords(request.message);
        }
    }
    
    std::expected<std::string, std::string> route_by_intent(const std::string& message)
    {
        // Use LLM to analyze intent and route to appropriate agent
        std::string routing_prompt = 
            "Analyze the following user request and determine which specialized agent should handle it.\n\n"
            "Available agents:\n";
        
        for (const auto& [name, agent] : agents_)
        {
            routing_prompt += "- " + name + "\n";
        }
        
        routing_prompt += "\nUser request: " + message + "\n\n";
        routing_prompt += "Respond with ONLY the agent name, nothing else.";
        
        CompletionConfig config;
        config.max_tokens = 50;
        config.temperature = 0.1;
        
        auto result = llm_client_->completion(routing_prompt, config);
        if (!result || result->choices.empty())
        {
            // Fallback to keyword routing
            return route_by_keywords(message);
        }
        
        std::string agent_name = result->choices[0].text;
        // Trim whitespace
        agent_name.erase(0, agent_name.find_first_not_of(" \t\n\r"));
        agent_name.erase(agent_name.find_last_not_of(" \t\n\r") + 1);
        
        // Validate agent exists
        if (agents_.count(agent_name))
        {
            return agent_name;
        }
        
        // Fallback to keyword routing
        return route_by_keywords(message);
    }
    
    std::expected<std::string, std::string> route_by_keywords(const std::string& message)
    {
        std::string lower_message = message;
        std::transform(lower_message.begin(), lower_message.end(), lower_message.begin(), ::tolower);
        
        // Code-related keywords
        if (lower_message.find("code") != std::string::npos ||
            lower_message.find("review") != std::string::npos ||
            lower_message.find("refactor") != std::string::npos ||
            lower_message.find("bug") != std::string::npos ||
            lower_message.find("function") != std::string::npos)
        {
            if (agents_.count("CodeAssistant"))
                return "CodeAssistant";
        }
        
        // DevOps keywords
        if (lower_message.find("deploy") != std::string::npos ||
            lower_message.find("service") != std::string::npos ||
            lower_message.find("log") != std::string::npos ||
            lower_message.find("infrastructure") != std::string::npos ||
            lower_message.find("monitoring") != std::string::npos)
        {
            if (agents_.count("DevOpsAgent"))
                return "DevOpsAgent";
        }
        
        // Documentation keywords
        if (lower_message.find("document") != std::string::npos ||
            lower_message.find("docs") != std::string::npos ||
            lower_message.find("api") != std::string::npos ||
            lower_message.find("readme") != std::string::npos)
        {
            if (agents_.count("DocumentationAgent"))
                return "DocumentationAgent";
        }
        
        // Testing keywords
        if (lower_message.find("test") != std::string::npos ||
            lower_message.find("coverage") != std::string::npos ||
            lower_message.find("unittest") != std::string::npos)
        {
            if (agents_.count("TestingAgent"))
                return "TestingAgent";
        }
        
        // Data analysis keywords
        if (lower_message.find("data") != std::string::npos ||
            lower_message.find("analyze") != std::string::npos ||
            lower_message.find("statistics") != std::string::npos ||
            lower_message.find("metrics") != std::string::npos)
        {
            if (agents_.count("DataAnalystAgent"))
                return "DataAnalystAgent";
        }
        
        // Security keywords
        if (lower_message.find("security") != std::string::npos ||
            lower_message.find("vulnerabilit") != std::string::npos ||
            lower_message.find("encryption") != std::string::npos ||
            lower_message.find("authentication") != std::string::npos)
        {
            if (agents_.count("SecurityAgent"))
                return "SecurityAgent";
        }
        
        // Default to first available agent
        if (!agents_.empty())
        {
            return agents_.begin()->first;
        }
        
        return std::unexpected("No agents available");
    }
    
    std::expected<std::string, std::string> route_round_robin()
    {
        if (agents_.empty())
        {
            return std::unexpected("No agents available");
        }
        
        auto it = agents_.begin();
        std::advance(it, round_robin_index_ % agents_.size());
        round_robin_index_++;
        
        return it->first;
    }
    
    // ========================================================================
    // Helper Methods
    // ========================================================================
    
    void register_default_agents()
    {
        // Register all 6 specialized agents
        register_agent("CodeAssistant", std::make_shared<CodeAssistant>(llm_endpoint_));
        register_agent("DevOpsAgent", std::make_shared<DevOpsAgent>(llm_endpoint_));
        register_agent("DocumentationAgent", std::make_shared<DocumentationAgent>(llm_endpoint_));
        register_agent("TestingAgent", std::make_shared<TestingAgent>(llm_endpoint_));
        register_agent("DataAnalystAgent", std::make_shared<DataAnalystAgent>(llm_endpoint_));
        register_agent("SecurityAgent", std::make_shared<SecurityAgent>(llm_endpoint_));
        
        if (verbose_)
        {
            std::cout << "[Coordinator] Registered 6 default specialized agents" << std::endl;
        }
    }
    
    bool should_suggest_followup(const std::string& message) const
    {
        // Check if message suggests multi-step workflow
        std::string lower = message;
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
        
        return lower.find("then") != std::string::npos ||
               lower.find("after") != std::string::npos ||
               lower.find("next") != std::string::npos ||
               lower.find("also") != std::string::npos;
    }
    
    std::optional<std::string> suggest_next_agent(const std::string& current_agent, const std::string& message)
    {
        // Simple heuristic: suggest related agent based on current one
        if (current_agent == "CodeAssistant" && message.find("test") != std::string::npos)
        {
            return "TestingAgent";
        }
        if (current_agent == "CodeAssistant" && message.find("deploy") != std::string::npos)
        {
            return "DevOpsAgent";
        }
        if (current_agent == "TestingAgent" && message.find("document") != std::string::npos)
        {
            return "DocumentationAgent";
        }
        
        return std::nullopt;
    }
    
    std::string aggregate_responses(const std::vector<std::string>& responses)
    {
        if (responses.size() == 1)
        {
            return responses[0];
        }
        
        std::string aggregated = "## Collaborative Response\n\n";
        aggregated += "Multiple agents have contributed to this response:\n\n";
        
        for (const auto& response : responses)
        {
            aggregated += "### " + response + "\n\n";
        }
        
        aggregated += "---\n\n";
        aggregated += "**Summary**: This response combines insights from " + 
                     std::to_string(responses.size()) + " specialized agents.";
        
        return aggregated;
    }

    std::string llm_endpoint_;
    std::shared_ptr<LlmClient> llm_client_;
    std::shared_ptr<McpServer> mcp_server_;
    std::map<std::string, std::shared_ptr<Agent>> agents_;
    std::map<std::string, Session> sessions_;
    std::map<std::string, int> agent_usage_;
    RoutingStrategy routing_strategy_;
    bool verbose_;
    size_t round_robin_index_;
};

} // namespace pooriayousefi

#endif // POORIAYOUSEFI_COORDINATOR_HPP
