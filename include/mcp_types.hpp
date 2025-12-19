#ifndef POORIAYOUSEFI_MCP_TYPES_HPP
#define POORIAYOUSEFI_MCP_TYPES_HPP

#include <string>
#include <optional>
#include <expected>
#include <variant>
#include <vector>
#include <map>
#include "../jsonrpc2/include/json.hpp"

namespace pooriayousefi
{

    // ============================================================================
    // JSON-RPC 2.0 Error Codes
    // ============================================================================

    enum class JsonRpcErrorCode
    {
        parse_error = -32700,
        invalid_request = -32600,
        method_not_found = -32601,
        invalid_params = -32602,
        internal_error = -32003,
        
        // MCP-specific error codes
        tool_not_found = -32001,
        tool_execution_failed = -32002,
        invalid_tool_params = -32003,
        server_not_initialized = -32004
    };

    // ============================================================================
    // JSON-RPC 2.0 Types
    // ============================================================================

    using JsonRpcId = std::variant<std::string, int, std::nullptr_t>;

    struct JsonRpcError
    {
        int code;
        std::string message;
        std::optional<nlohmann::json> data;

        JsonRpcError(int c, std::string msg, std::optional<nlohmann::json> d = std::nullopt)
            : code(c), message(std::move(msg)), data(std::move(d))
        {
        }

        JsonRpcError(JsonRpcErrorCode c, std::string msg, std::optional<nlohmann::json> d = std::nullopt)
            : code(static_cast<int>(c)), message(std::move(msg)), data(std::move(d))
        {
        }

        nlohmann::json to_json() const
        {
            nlohmann::json j;
            j["code"] = code;
            j["message"] = message;
            if (data)
            {
                j["data"] = *data;
            }
            return j;
        }

        static JsonRpcError from_json(const nlohmann::json& j)
        {
            int code = j.at("code").get<int>();
            std::string message = j.at("message").get<std::string>();
            std::optional<nlohmann::json> data;
            if (j.contains("data"))
            {
                data = j["data"];
            }
            return JsonRpcError(code, message, data);
        }
    };

    struct JsonRpcRequest
    {
        std::string jsonrpc{"2.0"};
        std::string method;
        std::optional<nlohmann::json> params;
        std::optional<JsonRpcId> id;

        nlohmann::json to_json() const
        {
            nlohmann::json j;
            j["jsonrpc"] = jsonrpc;
            j["method"] = method;
            
            if (params)
            {
                j["params"] = *params;
            }
            
            if (id)
            {
                std::visit([&j](auto&& arg)
                {
                    using T = std::decay_t<decltype(arg)>;
                    if constexpr (std::is_same_v<T, std::nullptr_t>)
                    {
                        j["id"] = nullptr;
                    }
                    else
                    {
                        j["id"] = arg;
                    }
                }, *id);
            }
            
            return j;
        }

        static std::expected<JsonRpcRequest, JsonRpcError> from_json(const nlohmann::json& j)
        {
            try
            {
                JsonRpcRequest req;
                
                if (!j.contains("jsonrpc") || j["jsonrpc"] != "2.0")
                {
                    return std::unexpected(JsonRpcError(
                        JsonRpcErrorCode::invalid_request,
                        "Invalid JSON-RPC version"
                    ));
                }
                
                if (!j.contains("method") || !j["method"].is_string())
                {
                    return std::unexpected(JsonRpcError(
                        JsonRpcErrorCode::invalid_request,
                        "Missing or invalid method field"
                    ));
                }
                
                req.jsonrpc = j["jsonrpc"];
                req.method = j["method"];
                
                if (j.contains("params"))
                {
                    req.params = j["params"];
                }
                
                if (j.contains("id"))
                {
                    if (j["id"].is_string())
                    {
                        req.id = j["id"].get<std::string>();
                    }
                    else if (j["id"].is_number_integer())
                    {
                        req.id = j["id"].get<int>();
                    }
                    else if (j["id"].is_null())
                    {
                        req.id = nullptr;
                    }
                }
                
                return req;
            }
            catch (const std::exception& e)
            {
                return std::unexpected(JsonRpcError(
                    JsonRpcErrorCode::parse_error,
                    std::string("Failed to parse request: ") + e.what()
                ));
            }
        }
    };

    struct JsonRpcResponse
    {
        std::string jsonrpc{"2.0"};
        std::optional<nlohmann::json> result;
        std::optional<JsonRpcError> error;
        JsonRpcId id;

        nlohmann::json to_json() const
        {
            nlohmann::json j;
            j["jsonrpc"] = jsonrpc;
            
            if (result)
            {
                j["result"] = *result;
            }
            
            if (error)
            {
                j["error"] = error->to_json();
            }
            
            std::visit([&j](auto&& arg)
            {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, std::nullptr_t>)
                {
                    j["id"] = nullptr;
                }
                else
                {
                    j["id"] = arg;
                }
            }, id);
            
            return j;
        }

        static JsonRpcResponse success(const nlohmann::json& result_data, JsonRpcId request_id)
        {
            JsonRpcResponse resp;
            resp.result = result_data;
            resp.id = request_id;
            return resp;
        }

        static JsonRpcResponse error_response(const JsonRpcError& err, JsonRpcId request_id)
        {
            JsonRpcResponse resp;
            resp.error = err;
            resp.id = request_id;
            return resp;
        }
    };

    // ============================================================================
    // MCP Protocol Types
    // ============================================================================

    struct McpServerInfo
    {
        std::string name;
        std::string version;

        nlohmann::json to_json() const
        {
            return nlohmann::json{
                {"name", name},
                {"version", version}
            };
        }
    };

    struct McpClientInfo
    {
        std::string name;
        std::string version;

        static McpClientInfo from_json(const nlohmann::json& j)
        {
            return McpClientInfo{
                j.value("name", "unknown"),
                j.value("version", "unknown")
            };
        }
    };

    struct McpCapabilities
    {
        bool tools{true};
        bool prompts{false};
        bool resources{false};
        bool logging{false};

        nlohmann::json to_json() const
        {
            nlohmann::json j;
            if (tools) j["tools"] = nlohmann::json::object();
            if (prompts) j["prompts"] = nlohmann::json::object();
            if (resources) j["resources"] = nlohmann::json::object();
            if (logging) j["logging"] = nlohmann::json::object();
            return j;
        }

        static McpCapabilities from_json(const nlohmann::json& j)
        {
            McpCapabilities caps;
            caps.tools = j.contains("tools");
            caps.prompts = j.contains("prompts");
            caps.resources = j.contains("resources");
            caps.logging = j.contains("logging");
            return caps;
        }
    };

    struct McpInitializeParams
    {
        std::string protocol_version{"2024-11-05"};
        McpClientInfo client_info;
        McpCapabilities capabilities;

        static McpInitializeParams from_json(const nlohmann::json& j)
        {
            McpInitializeParams params;
            params.protocol_version = j.value("protocolVersion", "2024-11-05");
            
            if (j.contains("clientInfo"))
            {
                params.client_info = McpClientInfo::from_json(j["clientInfo"]);
            }
            
            if (j.contains("capabilities"))
            {
                params.capabilities = McpCapabilities::from_json(j["capabilities"]);
            }
            
            return params;
        }
    };

    struct McpInitializeResult
    {
        std::string protocol_version{"2024-11-05"};
        McpServerInfo server_info;
        McpCapabilities capabilities;

        nlohmann::json to_json() const
        {
            return nlohmann::json{
                {"protocolVersion", protocol_version},
                {"serverInfo", server_info.to_json()},
                {"capabilities", capabilities.to_json()}
            };
        }
    };

    // ============================================================================
    // MCP Tool Types
    // ============================================================================

    struct McpToolParameter
    {
        std::string name;
        std::string type;  // "string", "number", "boolean", "object", "array"
        std::string description;
        bool required{true};
        std::optional<nlohmann::json> default_value;
        std::optional<std::vector<std::string>> enum_values;

        nlohmann::json to_json_schema() const
        {
            nlohmann::json schema;
            schema["type"] = type;
            schema["description"] = description;
            
            if (default_value)
            {
                schema["default"] = *default_value;
            }
            
            if (enum_values)
            {
                schema["enum"] = *enum_values;
            }
            
            return schema;
        }
    };

    struct McpTool
    {
        std::string name;
        std::string description;
        nlohmann::json input_schema;  // JSON Schema

        nlohmann::json to_json() const
        {
            return nlohmann::json{
                {"name", name},
                {"description", description},
                {"inputSchema", input_schema}
            };
        }

        // Helper to build input schema from parameters
        static nlohmann::json build_input_schema(const std::vector<McpToolParameter>& parameters)
        {
            nlohmann::json schema;
            schema["type"] = "object";
            schema["properties"] = nlohmann::json::object();
            schema["required"] = nlohmann::json::array();
            
            for (const auto& param : parameters)
            {
                schema["properties"][param.name] = param.to_json_schema();
                if (param.required)
                {
                    schema["required"].push_back(param.name);
                }
            }
            
            return schema;
        }
    };

    struct McpListToolsResult
    {
        std::vector<McpTool> tools;

        nlohmann::json to_json() const
        {
            nlohmann::json j;
            j["tools"] = nlohmann::json::array();
            for (const auto& tool : tools)
            {
                j["tools"].push_back(tool.to_json());
            }
            return j;
        }
    };

    struct McpCallToolParams
    {
        std::string name;
        nlohmann::json arguments;

        static McpCallToolParams from_json(const nlohmann::json& j)
        {
            McpCallToolParams params;
            params.name = j.at("name").get<std::string>();
            params.arguments = j.value("arguments", nlohmann::json::object());
            return params;
        }
    };

    struct McpToolResult
    {
        std::vector<nlohmann::json> content;
        bool is_error{false};

        nlohmann::json to_json() const
        {
            nlohmann::json j;
            j["content"] = content;
            if (is_error)
            {
                j["isError"] = true;
            }
            return j;
        }

        // Helper to create text content
        static McpToolResult text_result(const std::string& text, bool error = false)
        {
            McpToolResult result;
            result.content.push_back(nlohmann::json{
                {"type", "text"},
                {"text", text}
            });
            result.is_error = error;
            return result;
        }

        // Helper to create JSON content
        static McpToolResult json_result(const nlohmann::json& data, bool error = false)
        {
            McpToolResult result;
            result.content.push_back(nlohmann::json{
                {"type", "text"},
                {"text", data.dump(2)}
            });
            result.is_error = error;
            return result;
        }

        // Helper to create error result
        static McpToolResult error_result(const std::string& error_msg)
        {
            return text_result(error_msg, true);
        }
    };

    // ============================================================================
    // Validation Helpers
    // ============================================================================

    class McpValidator
    {
    public:
        // Validate tool parameters against JSON Schema
        static std::expected<bool, JsonRpcError> validate_tool_params(
            const nlohmann::json& params,
            const nlohmann::json& schema
        )
        {
            try
            {
                // Check if params is an object
                if (!params.is_object())
                {
                    return std::unexpected(JsonRpcError(
                        JsonRpcErrorCode::invalid_tool_params,
                        "Tool parameters must be an object"
                    ));
                }

                // Check required fields
                if (schema.contains("required") && schema["required"].is_array())
                {
                    for (const auto& req : schema["required"])
                    {
                        std::string field = req.get<std::string>();
                        if (!params.contains(field))
                        {
                            return std::unexpected(JsonRpcError(
                                JsonRpcErrorCode::invalid_tool_params,
                                "Missing required parameter: " + field
                            ));
                        }
                    }
                }

                // Validate types (basic validation)
                if (schema.contains("properties") && schema["properties"].is_object())
                {
                    for (auto it = params.begin(); it != params.end(); ++it)
                    {
                        const std::string& field_name = it.key();
                        
                        if (schema["properties"].contains(field_name))
                        {
                            const auto& field_schema = schema["properties"][field_name];
                            if (field_schema.contains("type"))
                            {
                                std::string expected_type = field_schema["type"].get<std::string>();
                                if (!validate_json_type(it.value(), expected_type))
                                {
                                    return std::unexpected(JsonRpcError(
                                        JsonRpcErrorCode::invalid_tool_params,
                                        "Invalid type for parameter '" + field_name + 
                                        "': expected " + expected_type
                                    ));
                                }
                            }
                        }
                    }
                }

                return true;
            }
            catch (const std::exception& e)
            {
                return std::unexpected(JsonRpcError(
                    JsonRpcErrorCode::invalid_tool_params,
                    std::string("Validation error: ") + e.what()
                ));
            }
        }

    private:
        static bool validate_json_type(const nlohmann::json& value, const std::string& type)
        {
            if (type == "string") return value.is_string();
            if (type == "number") return value.is_number();
            if (type == "integer") return value.is_number_integer();
            if (type == "boolean") return value.is_boolean();
            if (type == "object") return value.is_object();
            if (type == "array") return value.is_array();
            if (type == "null") return value.is_null();
            return true;  // Unknown type, allow it
        }
    };

    // ============================================================================
    // Utility Functions
    // ============================================================================

    inline JsonRpcId make_id(int id)
    {
        return id;
    }

    inline JsonRpcId make_id(const std::string& id)
    {
        return id;
    }

    inline JsonRpcId make_null_id()
    {
        return nullptr;
    }

}  // namespace pooriayousefi

#endif  // POORIAYOUSEFI_MCP_TYPES_HPP
