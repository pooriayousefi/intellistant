#ifndef POORIAYOUSEFI_AGENT_HPP
#define POORIAYOUSEFI_AGENT_HPP

#include "llm_client.hpp"
#include "mcp_server.hpp"
#include "mcp_tools.hpp"
#include <memory>
#include <vector>
#include <string>
#include <optional>
#include <expected>

namespace pooriayousefi
{

// ============================================================================
// Agent Configuration
// ============================================================================

struct AgentConfig
{
    std::string name;
    std::string version;
    std::string system_prompt;
    CompletionConfig llm_config;
    int max_tool_iterations = 10;  // Prevent infinite tool calling loops
    bool verbose = false;           // Enable debug output
};

// ============================================================================
// Agent Response
// ============================================================================

struct AgentResponse
{
    std::string content;
    std::vector<std::string> tool_calls_made;
    int iterations;
    bool stopped_by_limit;
    
    AgentResponse()
        : iterations(0), stopped_by_limit(false)
    {
    }
};

// ============================================================================
// Base Agent Class
// ============================================================================

class Agent
{
private:
    AgentConfig config_;
    std::shared_ptr<LlmClient> llm_client_;
    std::shared_ptr<McpServer> mcp_server_;
    std::shared_ptr<McpClient> mcp_client_;
    std::vector<ChatMessage> conversation_history_;
    
public:
    // Constructor
    Agent(
        const AgentConfig& config,
        const std::string& llm_server_url
    )
        : config_(config)
        , llm_client_(std::make_shared<LlmClient>(llm_server_url))
        , mcp_server_(std::make_shared<McpServer>(config.name, config.version))
        , mcp_client_(std::make_shared<McpClient>(mcp_server_))
    {
        // Initialize MCP client
        auto init_result = mcp_client_->initialize(config.name, config.version);
        if (!init_result)
        {
            throw std::runtime_error("Failed to initialize MCP client: " + init_result.error().message);
        }
        
        // Register all available tools
        ToolRegistry::register_filesystem_tools(*mcp_server_);
        ToolRegistry::register_git_tools(*mcp_server_);
        ToolRegistry::register_system_tools(*mcp_server_);
        
        // Initialize conversation with system prompt
        if (!config.system_prompt.empty())
        {
            conversation_history_.push_back({ChatRole::System, config.system_prompt});
        }
    }
    
    // Get agent configuration
    const AgentConfig& get_config() const
    {
        return config_;
    }
    
    // Get conversation history
    const std::vector<ChatMessage>& get_conversation_history() const
    {
        return conversation_history_;
    }
    
    // Clear conversation history (keeps system prompt)
    void clear_conversation()
    {
        std::vector<ChatMessage> system_messages;
        for (const auto& msg : conversation_history_)
        {
            if (msg.role == ChatRole::System)
            {
                system_messages.push_back(msg);
            }
        }
        conversation_history_ = std::move(system_messages);
    }
    
    // Get available tools as function schemas
    std::vector<nlohmann::json> get_function_schemas() const
    {
        return mcp_server_->get_function_schemas();
    }
    
    // Process a user message with tool calling support
    std::expected<AgentResponse, std::string> process(const std::string& user_message)
    {
        if (config_.verbose)
        {
            std::cout << "[Agent] Processing user message: " << user_message << std::endl;
        }
        
        // Add user message to history
        conversation_history_.push_back({ChatRole::User, user_message});
        
        AgentResponse response;
        
        // Tool calling loop
        for (int iteration = 0; iteration < config_.max_tool_iterations; ++iteration)
        {
            response.iterations = iteration + 1;
            
            if (config_.verbose)
            {
                std::cout << "[Agent] Iteration " << response.iterations << std::endl;
            }
            
            // Get function schemas for LLM
            auto functions = get_function_schemas();
            
            // Call LLM with conversation history and available functions
            auto chat_result = llm_client_->chat_completion_with_tools(
                conversation_history_,
                functions,
                config_.llm_config
            );
            
            if (!chat_result)
            {
                return std::unexpected("LLM error: " + chat_result.error().message);
            }
            
            // Check if LLM wants to call a tool
            if (chat_result->tool_calls.empty())
            {
                // No tool calls - this is the final response
                response.content = chat_result->content;
                conversation_history_.push_back({ChatRole::Assistant, chat_result->content});
                
                if (config_.verbose)
                {
                    std::cout << "[Agent] Final response: " << response.content << std::endl;
                }
                
                return response;
            }
            
            // LLM wants to call tools
            if (config_.verbose)
            {
                std::cout << "[Agent] LLM requested " << chat_result->tool_calls.size() << " tool call(s)" << std::endl;
            }
            
            // Add assistant message with tool calls to history
            conversation_history_.push_back({
                ChatRole::Assistant,
                chat_result->content,
                chat_result->tool_calls
            });
            
            // Execute each tool call
            for (const auto& tool_call : chat_result->tool_calls)
            {
                if (config_.verbose)
                {
                    std::cout << "[Agent] Calling tool: " << tool_call.function.name << std::endl;
                    std::cout << "[Agent] Arguments: " << tool_call.function.arguments.dump() << std::endl;
                }
                
                // Execute tool
                auto tool_result = mcp_client_->call_tool(
                    tool_call.function.name,
                    tool_call.function.arguments
                );
                
                std::string result_content;
                if (tool_result)
                {
                    // Format tool result as JSON string
                    nlohmann::json result_json;
                    result_json["success"] = true;
                    result_json["content"] = tool_result->content;
                    result_content = result_json.dump();
                    
                    if (config_.verbose)
                    {
                        std::cout << "[Agent] Tool result: " << result_content << std::endl;
                    }
                }
                else
                {
                    // Tool execution failed
                    nlohmann::json error_json;
                    error_json["success"] = false;
                    error_json["error"] = tool_result.error().message;
                    result_content = error_json.dump();
                    
                    if (config_.verbose)
                    {
                        std::cout << "[Agent] Tool error: " << result_content << std::endl;
                    }
                }
                
                // Add tool result to conversation history
                conversation_history_.push_back({
                    ChatRole::Tool,
                    result_content,
                    {},
                    tool_call.id
                });
                
                response.tool_calls_made.push_back(tool_call.function.name);
            }
        }
        
        // Hit iteration limit
        response.stopped_by_limit = true;
        response.content = "Maximum tool iterations reached. Please simplify your request.";
        
        if (config_.verbose)
        {
            std::cout << "[Agent] Hit iteration limit" << std::endl;
        }
        
        return response;
    }
    
    // Add custom system instruction
    void add_system_instruction(const std::string& instruction)
    {
        conversation_history_.push_back({ChatRole::System, instruction});
    }
    
    // Register custom tool
    void register_tool(
        const std::string& name,
        const std::string& description,
        const std::vector<McpToolParameter>& parameters,
        ToolHandler function
    )
    {
        mcp_server_->register_tool(name, description, parameters, function);
    }
    
    // Get MCP server (for advanced usage)
    std::shared_ptr<McpServer> get_mcp_server()
    {
        return mcp_server_;
    }
    
    // Get LLM client (for advanced usage)
    std::shared_ptr<LlmClient> get_llm_client()
    {
        return llm_client_;
    }
};

// ============================================================================
// Agent Factory
// ============================================================================

class AgentFactory
{
public:
    static std::shared_ptr<Agent> create_agent(
        const std::string& name,
        const std::string& system_prompt,
        const std::string& llm_server_url = "http://localhost:8080",
        const CompletionConfig& llm_config = CompletionConfig{}
    )
    {
        AgentConfig config;
        config.name = name;
        config.version = "1.0.0";
        config.system_prompt = system_prompt;
        config.llm_config = llm_config;
        config.max_tool_iterations = 10;
        config.verbose = false;
        
        return std::make_shared<Agent>(config, llm_server_url);
    }
};

} // namespace pooriayousefi

#endif // POORIAYOUSEFI_AGENT_HPP
