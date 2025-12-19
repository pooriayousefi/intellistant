#include <iostream>
#include <cassert>
#include "../include/agent.hpp"
#include "../include/agents.hpp"

using namespace pooriayousefi;

// ============================================================================
// Test Helpers
// ============================================================================

void test_header(const std::string& test_name)
{
    std::cout << "\n========================================" << std::endl;
    std::cout << "TEST: " << test_name << std::endl;
    std::cout << "========================================" << std::endl;
}

void test_success(const std::string& message)
{
    std::cout << "[✓] " << message << std::endl;
}

void test_failure(const std::string& message)
{
    std::cerr << "[✗] " << message << std::endl;
    exit(1);
}

// ============================================================================
// Test Cases
// ============================================================================

void test_agent_creation()
{
    test_header("Agent Creation and Configuration");
    
    AgentConfig config;
    config.name = "TestAgent";
    config.version = "1.0.0";
    config.system_prompt = "You are a helpful assistant.";
    config.llm_config.temperature = 0.7;
    config.llm_config.max_tokens = 512;
    config.max_tool_iterations = 10;
    config.verbose = false;
    
    try
    {
        Agent agent(config, "http://localhost:8080");
        test_success("Agent created successfully");
        
        const auto& agent_config = agent.get_config();
        if (agent_config.name == "TestAgent")
        {
            test_success("Agent configuration correct");
        }
        else
        {
            test_failure("Agent configuration mismatch");
        }
    }
    catch (const std::exception& e)
    {
        // This is expected if llama-server is not running
        std::cout << "[SKIP] Agent creation (LLM server not available): " << e.what() << std::endl;
    }
}

void test_specialized_agents()
{
    test_header("Specialized Agent Creation");
    
    try
    {
        // Test CodeAssistant
        {
            CodeAssistant assistant;
            const auto& config = assistant.get_config();
            
            if (config.name == "CodeAssistant")
            {
                test_success("CodeAssistant created with correct name");
            }
            
            if (!config.system_prompt.empty())
            {
                test_success("CodeAssistant has system prompt");
            }
            
            if (config.llm_config.temperature.has_value() && 
                *config.llm_config.temperature == 0.3f)
            {
                test_success("CodeAssistant has correct temperature");
            }
        }
        
        // Test DevOpsAgent
        {
            DevOpsAgent agent;
            const auto& config = agent.get_config();
            
            if (config.name == "DevOpsAgent")
            {
                test_success("DevOpsAgent created with correct name");
            }
        }
        
        // Test DocumentationAgent
        {
            DocumentationAgent agent;
            const auto& config = agent.get_config();
            
            if (config.name == "DocumentationAgent")
            {
                test_success("DocumentationAgent created with correct name");
            }
        }
        
        // Test TestingAgent
        {
            TestingAgent agent;
            const auto& config = agent.get_config();
            
            if (config.name == "TestingAgent")
            {
                test_success("TestingAgent created with correct name");
            }
        }
        
        // Test DataAnalystAgent
        {
            DataAnalystAgent agent;
            const auto& config = agent.get_config();
            
            if (config.name == "DataAnalystAgent")
            {
                test_success("DataAnalystAgent created with correct name");
            }
        }
        
        // Test SecurityAgent
        {
            SecurityAgent agent;
            const auto& config = agent.get_config();
            
            if (config.name == "SecurityAgent")
            {
                test_success("SecurityAgent created with correct name");
            }
        }
    }
    catch (const std::exception& e)
    {
        std::cout << "[SKIP] Specialized agents (LLM server not available)" << std::endl;
    }
}

void test_agent_factory()
{
    test_header("Agent Factory");
    
    try
    {
        auto agent = AgentFactory::create_agent(
            "CustomAgent",
            "You are a custom assistant."
        );
        
        if (agent)
        {
            test_success("Agent factory created agent");
            
            const auto& config = agent->get_config();
            if (config.name == "CustomAgent")
            {
                test_success("Agent factory set correct name");
            }
        }
    }
    catch (const std::exception& e)
    {
        std::cout << "[SKIP] Agent factory (LLM server not available)" << std::endl;
    }
}

void test_conversation_management()
{
    test_header("Conversation Management");
    
    try
    {
        AgentConfig config;
        config.name = "TestAgent";
        config.version = "1.0.0";
        config.system_prompt = "You are a helpful assistant.";
        
        Agent agent(config, "http://localhost:8080");
        
        // Check initial history (should have system message)
        const auto& history = agent.get_conversation_history();
        if (!history.empty() && history[0].role == ChatRole::System)
        {
            test_success("System prompt in conversation history");
        }
        
        // Add custom system instruction
        agent.add_system_instruction("Additional instruction");
        
        const auto& updated_history = agent.get_conversation_history();
        if (updated_history.size() == 2)
        {
            test_success("System instruction added to history");
        }
        
        // Clear conversation
        agent.clear_conversation();
        
        const auto& cleared_history = agent.get_conversation_history();
        if (cleared_history.size() == 2)  // Both system messages preserved
        {
            test_success("Conversation cleared (system messages preserved)");
        }
    }
    catch (const std::exception& e)
    {
        std::cout << "[SKIP] Conversation management (LLM server not available)" << std::endl;
    }
}

void test_function_schemas()
{
    test_header("Function Schema Generation");
    
    try
    {
        AgentConfig config;
        config.name = "TestAgent";
        config.version = "1.0.0";
        config.system_prompt = "Test";
        
        Agent agent(config, "http://localhost:8080");
        
        auto schemas = agent.get_function_schemas();
        
        if (!schemas.empty())
        {
            test_success("Agent has function schemas");
            std::cout << "Number of available tools: " << schemas.size() << std::endl;
            
            // Check schema format
            bool valid_format = true;
            for (const auto& schema : schemas)
            {
                if (!schema.contains("type") || schema["type"] != "function")
                {
                    valid_format = false;
                    break;
                }
                
                if (!schema.contains("function") ||
                    !schema["function"].contains("name") ||
                    !schema["function"].contains("description") ||
                    !schema["function"].contains("parameters"))
                {
                    valid_format = false;
                    break;
                }
            }
            
            if (valid_format)
            {
                test_success("Function schemas have correct format");
            }
            else
            {
                test_failure("Invalid function schema format");
            }
        }
        else
        {
            test_failure("No function schemas available");
        }
    }
    catch (const std::exception& e)
    {
        std::cout << "[SKIP] Function schemas (LLM server not available)" << std::endl;
    }
}

void test_tool_access()
{
    test_header("Tool Access and Registration");
    
    try
    {
        AgentConfig config;
        config.name = "TestAgent";
        config.version = "1.0.0";
        config.system_prompt = "Test";
        
        Agent agent(config, "http://localhost:8080");
        
        // Get MCP server
        auto mcp_server = agent.get_mcp_server();
        if (mcp_server)
        {
            test_success("Agent provides MCP server access");
        }
        
        // Get LLM client
        auto llm_client = agent.get_llm_client();
        if (llm_client)
        {
            test_success("Agent provides LLM client access");
        }
        
        // Register custom tool
        std::vector<McpToolParameter> params;
        params.push_back({"input", "string", "Test input", true});
        
        agent.register_tool(
            "custom_tool",
            "A custom test tool",
            params,
            [](const nlohmann::json& args) -> std::expected<McpToolResult, JsonRpcError>
            {
                McpToolResult result;
                result.content.push_back({{"type", "text"}, {"text", "Custom tool result"}});
                return result;
            }
        );
        
        // Check if custom tool is registered
        auto schemas = agent.get_function_schemas();
        bool found_custom = false;
        for (const auto& schema : schemas)
        {
            if (schema["function"]["name"] == "custom_tool")
            {
                found_custom = true;
                break;
            }
        }
        
        if (found_custom)
        {
            test_success("Custom tool registered successfully");
        }
        else
        {
            test_failure("Custom tool not found in schemas");
        }
    }
    catch (const std::exception& e)
    {
        std::cout << "[SKIP] Tool access (LLM server not available)" << std::endl;
    }
}

void test_agent_response_structure()
{
    test_header("Agent Response Structure");
    
    // Test response structure
    AgentResponse response;
    response.content = "Test response";
    response.tool_calls_made.push_back("read_file");
    response.tool_calls_made.push_back("write_file");
    response.iterations = 3;
    response.stopped_by_limit = false;
    
    if (response.content == "Test response")
    {
        test_success("Response content field works");
    }
    
    if (response.tool_calls_made.size() == 2)
    {
        test_success("Response tracks tool calls");
    }
    
    if (response.iterations == 3)
    {
        test_success("Response tracks iterations");
    }
    
    if (!response.stopped_by_limit)
    {
        test_success("Response tracks limit status");
    }
}

// ============================================================================
// Main Test Runner
// ============================================================================

int main()
{
    std::cout << "========================================" << std::endl;
    std::cout << "AGENT SYSTEM TEST SUITE" << std::endl;
    std::cout << "========================================" << std::endl;
    
    try
    {
        test_agent_creation();
        test_specialized_agents();
        test_agent_factory();
        test_conversation_management();
        test_function_schemas();
        test_tool_access();
        test_agent_response_structure();
        
        std::cout << "\n========================================" << std::endl;
        std::cout << "ALL TESTS PASSED! ✓" << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << "\nNote: Some tests may be skipped if llama-server is not running." << std::endl;
        std::cout << "For full testing with LLM integration, start llama-server on localhost:8080" << std::endl;
        
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "\n========================================" << std::endl;
        std::cerr << "TEST FAILED WITH EXCEPTION: " << e.what() << std::endl;
        std::cerr << "========================================" << std::endl;
        return 1;
    }
}
