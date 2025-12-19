#ifndef POORIAYOUSEFI_MCP_SERVER_HPP
#define POORIAYOUSEFI_MCP_SERVER_HPP

#include <string>
#include <map>
#include <memory>
#include <functional>
#include <mutex>
#include "mcp_types.hpp"

namespace pooriayousefi
{

    // ============================================================================
    // Tool Handler Type
    // ============================================================================

    using ToolHandler = std::function<std::expected<McpToolResult, JsonRpcError>(const nlohmann::json&)>;

    // ============================================================================
    // MCP Server
    // ============================================================================

    class McpServer
    {
    public:
        McpServer(const std::string& name, const std::string& version)
            : server_info_{name, version}
            , initialized_(false)
        {
            capabilities_.tools = true;
            capabilities_.prompts = false;
            capabilities_.resources = false;
            capabilities_.logging = false;
        }

        // ========================================================================
        // Tool Registration
        // ========================================================================

        void register_tool(
            const std::string& name,
            const std::string& description,
            const nlohmann::json& input_schema,
            ToolHandler handler
        )
        {
            std::lock_guard<std::mutex> lock(mutex_);
            
            McpTool tool;
            tool.name = name;
            tool.description = description;
            tool.input_schema = input_schema;
            
            tools_[name] = tool;
            tool_handlers_[name] = std::move(handler);
        }

        void register_tool(
            const std::string& name,
            const std::string& description,
            const std::vector<McpToolParameter>& parameters,
            ToolHandler handler
        )
        {
            auto schema = McpTool::build_input_schema(parameters);
            register_tool(name, description, schema, std::move(handler));
        }

        void unregister_tool(const std::string& name)
        {
            std::lock_guard<std::mutex> lock(mutex_);
            tools_.erase(name);
            tool_handlers_.erase(name);
        }

        std::vector<std::string> list_tool_names() const
        {
            std::lock_guard<std::mutex> lock(mutex_);
            std::vector<std::string> names;
            for (const auto& [name, _] : tools_)
            {
                names.push_back(name);
            }
            return names;
        }

        // ========================================================================
        // JSON-RPC 2.0 Message Handling
        // ========================================================================

        nlohmann::json handle_request(const std::string& request_str)
        {
            try
            {
                auto request_json = nlohmann::json::parse(request_str);
                return handle_request(request_json);
            }
            catch (const std::exception& e)
            {
                auto error = JsonRpcError(
                    JsonRpcErrorCode::parse_error,
                    std::string("JSON parse error: ") + e.what()
                );
                return JsonRpcResponse::error_response(error, make_null_id()).to_json();
            }
        }

        nlohmann::json handle_request(const nlohmann::json& request_json)
        {
            // Parse JSON-RPC request
            auto request_result = JsonRpcRequest::from_json(request_json);
            
            if (!request_result)
            {
                return JsonRpcResponse::error_response(
                    request_result.error(),
                    make_null_id()
                ).to_json();
            }
            
            const auto& request = request_result.value();
            
            // Determine request ID for response
            JsonRpcId response_id = request.id ? *request.id : make_null_id();
            
            // Route to appropriate handler
            std::expected<nlohmann::json, JsonRpcError> result;
            
            if (request.method == "initialize")
            {
                result = handle_initialize(request.params);
            }
            else if (request.method == "tools/list")
            {
                result = handle_list_tools(request.params);
            }
            else if (request.method == "tools/call")
            {
                result = handle_call_tool(request.params);
            }
            else if (request.method == "ping")
            {
                result = handle_ping(request.params);
            }
            else
            {
                result = std::unexpected(JsonRpcError(
                    JsonRpcErrorCode::method_not_found,
                    "Method not found: " + request.method
                ));
            }
            
            // Build response
            if (result)
            {
                return JsonRpcResponse::success(result.value(), response_id).to_json();
            }
            else
            {
                return JsonRpcResponse::error_response(result.error(), response_id).to_json();
            }
        }

        // ========================================================================
        // MCP Protocol Handlers
        // ========================================================================

        std::expected<nlohmann::json, JsonRpcError> handle_initialize(
            const std::optional<nlohmann::json>& params
        )
        {
            if (!params)
            {
                return std::unexpected(JsonRpcError(
                    JsonRpcErrorCode::invalid_params,
                    "Initialize requires parameters"
                ));
            }

            try
            {
                auto init_params = McpInitializeParams::from_json(*params);
                
                // Store client info
                client_info_ = init_params.client_info;
                
                // Mark as initialized
                initialized_ = true;
                
                // Build result
                McpInitializeResult result;
                result.protocol_version = "2024-11-05";
                result.server_info = server_info_;
                result.capabilities = capabilities_;
                
                return result.to_json();
            }
            catch (const std::exception& e)
            {
                return std::unexpected(JsonRpcError(
                    JsonRpcErrorCode::invalid_params,
                    std::string("Invalid initialize params: ") + e.what()
                ));
            }
        }

        std::expected<nlohmann::json, JsonRpcError> handle_list_tools(
            const std::optional<nlohmann::json>& params
        )
        {
            if (!initialized_)
            {
                return std::unexpected(JsonRpcError(
                    JsonRpcErrorCode::server_not_initialized,
                    "Server not initialized. Call 'initialize' first."
                ));
            }

            std::lock_guard<std::mutex> lock(mutex_);
            
            McpListToolsResult result;
            for (const auto& [name, tool] : tools_)
            {
                result.tools.push_back(tool);
            }
            
            return result.to_json();
        }

        std::expected<nlohmann::json, JsonRpcError> handle_call_tool(
            const std::optional<nlohmann::json>& params
        )
        {
            if (!initialized_)
            {
                return std::unexpected(JsonRpcError(
                    JsonRpcErrorCode::server_not_initialized,
                    "Server not initialized. Call 'initialize' first."
                ));
            }

            if (!params)
            {
                return std::unexpected(JsonRpcError(
                    JsonRpcErrorCode::invalid_params,
                    "Tool call requires parameters"
                ));
            }

            try
            {
                auto call_params = McpCallToolParams::from_json(*params);
                
                // Find tool
                std::lock_guard<std::mutex> lock(mutex_);
                
                auto tool_it = tools_.find(call_params.name);
                if (tool_it == tools_.end())
                {
                    return std::unexpected(JsonRpcError(
                        JsonRpcErrorCode::tool_not_found,
                        "Tool not found: " + call_params.name
                    ));
                }
                
                auto handler_it = tool_handlers_.find(call_params.name);
                if (handler_it == tool_handlers_.end())
                {
                    return std::unexpected(JsonRpcError(
                        JsonRpcErrorCode::internal_error,
                        "Tool handler not found: " + call_params.name
                    ));
                }
                
                const auto& tool = tool_it->second;
                const auto& handler = handler_it->second;
                
                // Validate parameters
                auto validation = McpValidator::validate_tool_params(
                    call_params.arguments,
                    tool.input_schema
                );
                
                if (!validation)
                {
                    return std::unexpected(validation.error());
                }
                
                // Execute tool
                auto result = handler(call_params.arguments);
                
                if (!result)
                {
                    return std::unexpected(result.error());
                }
                
                return result.value().to_json();
            }
            catch (const std::exception& e)
            {
                return std::unexpected(JsonRpcError(
                    JsonRpcErrorCode::tool_execution_failed,
                    std::string("Tool execution failed: ") + e.what()
                ));
            }
        }

        std::expected<nlohmann::json, JsonRpcError> handle_ping(
            const std::optional<nlohmann::json>& params
        )
        {
            return nlohmann::json::object();
        }

        // ========================================================================
        // LLM Function Calling Format
        // ========================================================================

        std::vector<nlohmann::json> get_function_schemas() const
        {
            std::lock_guard<std::mutex> lock(mutex_);
            
            std::vector<nlohmann::json> schemas;
            
            for (const auto& [name, tool] : tools_)
            {
                nlohmann::json schema;
                schema["type"] = "function";
                schema["function"] = nlohmann::json{
                    {"name", tool.name},
                    {"description", tool.description},
                    {"parameters", tool.input_schema}
                };
                schemas.push_back(schema);
            }
            
            return schemas;
        }

        // ========================================================================
        // Server State
        // ========================================================================

        bool is_initialized() const
        {
            return initialized_;
        }

        McpServerInfo get_server_info() const
        {
            return server_info_;
        }

        McpCapabilities get_capabilities() const
        {
            return capabilities_;
        }

    private:
        McpServerInfo server_info_;
        McpCapabilities capabilities_;
        std::optional<McpClientInfo> client_info_;
        bool initialized_;
        
        std::map<std::string, McpTool> tools_;
        std::map<std::string, ToolHandler> tool_handlers_;
        
        mutable std::mutex mutex_;
    };

    // ============================================================================
    // MCP Client (for tool invocation from agents)
    // ============================================================================

    class McpClient
    {
    public:
        explicit McpClient(std::shared_ptr<McpServer> server)
            : server_(std::move(server))
            , next_id_(1)
        {
        }

        // Initialize the connection
        std::expected<McpInitializeResult, JsonRpcError> initialize(
            const std::string& client_name,
            const std::string& client_version
        )
        {
            McpClientInfo client_info{client_name, client_version};
            McpCapabilities capabilities;
            capabilities.tools = true;
            
            nlohmann::json params;
            params["protocolVersion"] = "2024-11-05";
            params["clientInfo"] = nlohmann::json{
                {"name", client_info.name},
                {"version", client_info.version}
            };
            params["capabilities"] = capabilities.to_json();
            
            auto result = send_request("initialize", params);
            
            if (!result)
            {
                return std::unexpected(result.error());
            }
            
            try
            {
                McpInitializeResult init_result;
                init_result.protocol_version = result->value("protocolVersion", "2024-11-05");
                
                if (result->contains("serverInfo"))
                {
                    auto& si = (*result)["serverInfo"];
                    init_result.server_info.name = si.value("name", "unknown");
                    init_result.server_info.version = si.value("version", "unknown");
                }
                
                if (result->contains("capabilities"))
                {
                    init_result.capabilities = McpCapabilities::from_json((*result)["capabilities"]);
                }
                
                return init_result;
            }
            catch (const std::exception& e)
            {
                return std::unexpected(JsonRpcError(
                    JsonRpcErrorCode::internal_error,
                    std::string("Failed to parse initialize result: ") + e.what()
                ));
            }
        }

        // List available tools
        std::expected<std::vector<McpTool>, JsonRpcError> list_tools()
        {
            auto result = send_request("tools/list", nlohmann::json::object());
            
            if (!result)
            {
                return std::unexpected(result.error());
            }
            
            try
            {
                std::vector<McpTool> tools;
                
                if (result->contains("tools") && (*result)["tools"].is_array())
                {
                    for (const auto& tool_json : (*result)["tools"])
                    {
                        McpTool tool;
                        tool.name = tool_json.at("name").get<std::string>();
                        tool.description = tool_json.at("description").get<std::string>();
                        tool.input_schema = tool_json.at("inputSchema");
                        tools.push_back(tool);
                    }
                }
                
                return tools;
            }
            catch (const std::exception& e)
            {
                return std::unexpected(JsonRpcError(
                    JsonRpcErrorCode::internal_error,
                    std::string("Failed to parse tools list: ") + e.what()
                ));
            }
        }

        // Call a tool
        std::expected<McpToolResult, JsonRpcError> call_tool(
            const std::string& tool_name,
            const nlohmann::json& arguments
        )
        {
            nlohmann::json params;
            params["name"] = tool_name;
            params["arguments"] = arguments;
            
            auto result = send_request("tools/call", params);
            
            if (!result)
            {
                return std::unexpected(result.error());
            }
            
            try
            {
                McpToolResult tool_result;
                
                if (result->contains("content") && (*result)["content"].is_array())
                {
                    tool_result.content = (*result)["content"].get<std::vector<nlohmann::json>>();
                }
                
                if (result->contains("isError"))
                {
                    tool_result.is_error = (*result)["isError"].get<bool>();
                }
                
                return tool_result;
            }
            catch (const std::exception& e)
            {
                return std::unexpected(JsonRpcError(
                    JsonRpcErrorCode::internal_error,
                    std::string("Failed to parse tool result: ") + e.what()
                ));
            }
        }

    private:
        std::expected<nlohmann::json, JsonRpcError> send_request(
            const std::string& method,
            const nlohmann::json& params
        )
        {
            JsonRpcRequest request;
            request.method = method;
            request.params = params;
            request.id = next_id_++;
            
            auto response_json = server_->handle_request(request.to_json());
            
            // Parse response
            try
            {
                if (response_json.contains("error"))
                {
                    return std::unexpected(JsonRpcError::from_json(response_json["error"]));
                }
                
                if (response_json.contains("result"))
                {
                    return response_json["result"];
                }
                
                return std::unexpected(JsonRpcError(
                    JsonRpcErrorCode::internal_error,
                    "Invalid response: missing result or error"
                ));
            }
            catch (const std::exception& e)
            {
                return std::unexpected(JsonRpcError(
                    JsonRpcErrorCode::internal_error,
                    std::string("Failed to parse response: ") + e.what()
                ));
            }
        }

        std::shared_ptr<McpServer> server_;
        int next_id_;
    };

}  // namespace pooriayousefi

#endif  // POORIAYOUSEFI_MCP_SERVER_HPP
