#include <iostream>
#include "../include/mcp_server.hpp"
#include "../include/mcp_tools.hpp"

using namespace pooriayousefi;

void print_separator()
{
    std::cout << "\n" << std::string(60, '=') << "\n" << std::endl;
}

int main()
{
    std::cout << "========================================" << std::endl;
    std::cout << "INTELLISTANT FRAMEWORK DEMO" << std::endl;
    std::cout << "MCP-Based Tool System" << std::endl;
    std::cout << "========================================" << std::endl;
    
    // Initialize MCP server
    print_separator();
    std::cout << "1. Creating MCP Server..." << std::endl;
    auto server = std::make_shared<McpServer>("Intellistant", "1.0.0");
    std::cout << "   ✓ Server created" << std::endl;
    
    // Register tools
    print_separator();
    std::cout << "2. Registering Tools..." << std::endl;
    ToolRegistry::register_filesystem_tools(*server);
    ToolRegistry::register_git_tools(*server);
    ToolRegistry::register_system_tools(*server);
    std::cout << "   ✓ All tools registered" << std::endl;
    
    // Create client
    print_separator();
    std::cout << "3. Creating MCP Client..." << std::endl;
    auto client = std::make_shared<McpClient>(server);
    
    auto init_result = client->initialize("DemoAgent", "1.0.0");
    if (init_result)
    {
        std::cout << "   ✓ Client initialized" << std::endl;
        std::cout << "   Server: " << init_result->server_info.name 
                  << " v" << init_result->server_info.version << std::endl;
        std::cout << "   Protocol: " << init_result->protocol_version << std::endl;
    }
    
    // List available tools
    print_separator();
    std::cout << "4. Available Tools:" << std::endl;
    auto tools = client->list_tools();
    if (tools)
    {
        for (const auto& tool : *tools)
        {
            std::cout << "   • " << tool.name << std::endl;
            std::cout << "     " << tool.description << std::endl;
        }
    }
    
    // Demo: File operations
    print_separator();
    std::cout << "5. Demo: File Operations" << std::endl;
    
    // Create a test file
    {
        std::cout << "\n   Creating test file..." << std::endl;
        nlohmann::json args;
        args["path"] = "/tmp/intellistant_demo.txt";
        args["content"] = "Hello from Intellistant Framework!\n\nThis is a demo of the MCP-based tool system.";
        
        auto result = client->call_tool("write_file", args);
        if (result)
        {
            std::cout << "   ✓ File created: /tmp/intellistant_demo.txt" << std::endl;
        }
    }
    
    // Read the file back
    {
        std::cout << "\n   Reading file back..." << std::endl;
        nlohmann::json args;
        args["path"] = "/tmp/intellistant_demo.txt";
        
        auto result = client->call_tool("read_file", args);
        if (result && !result->content.empty())
        {
            std::string content = result->content[0]["text"];
            std::cout << "   ✓ File content:" << std::endl;
            std::cout << "   " << std::string(50, '-') << std::endl;
            std::cout << "   " << content << std::endl;
            std::cout << "   " << std::string(50, '-') << std::endl;
        }
    }
    
    // Get file info
    {
        std::cout << "\n   Getting file metadata..." << std::endl;
        nlohmann::json args;
        args["path"] = "/tmp/intellistant_demo.txt";
        
        auto result = client->call_tool("file_info", args);
        if (result && !result->content.empty())
        {
            std::cout << "   ✓ File info retrieved" << std::endl;
        }
    }
    
    // Demo: System commands
    print_separator();
    std::cout << "6. Demo: System Commands" << std::endl;
    
    {
        std::cout << "\n   Executing: date" << std::endl;
        nlohmann::json args;
        args["command"] = "date";
        
        auto result = client->call_tool("execute_command", args);
        if (result && !result->content.empty())
        {
            std::string output = result->content[0]["text"];
            std::cout << "   Output: " << output;
        }
    }
    
    {
        std::cout << "\n   Executing: uname -a" << std::endl;
        nlohmann::json args;
        args["command"] = "uname -a";
        
        auto result = client->call_tool("execute_command", args);
        if (result && !result->content.empty())
        {
            std::string output = result->content[0]["text"];
            std::cout << "   Output: " << output;
        }
    }
    
    // Demo: LLM Function Calling Format
    print_separator();
    std::cout << "7. Demo: LLM Function Calling Format" << std::endl;
    
    auto schemas = server->get_function_schemas();
    std::cout << "\n   Generated " << schemas.size() << " function schemas for LLM" << std::endl;
    std::cout << "\n   Example schema (read_file):" << std::endl;
    std::cout << "   " << std::string(50, '-') << std::endl;
    
    for (const auto& schema : schemas)
    {
        if (schema["function"]["name"] == "read_file")
        {
            std::cout << "   " << schema.dump(2) << std::endl;
            break;
        }
    }
    std::cout << "   " << std::string(50, '-') << std::endl;
    
    // Demo: Error handling
    print_separator();
    std::cout << "8. Demo: Error Handling" << std::endl;
    
    {
        std::cout << "\n   Trying to read non-existent file..." << std::endl;
        nlohmann::json args;
        args["path"] = "/nonexistent/file.txt";
        
        auto result = client->call_tool("read_file", args);
        if (!result)
        {
            std::cout << "   ✓ Error caught correctly:" << std::endl;
            std::cout << "     Code: " << result.error().code << std::endl;
            std::cout << "     Message: " << result.error().message << std::endl;
        }
    }
    
    // Summary
    print_separator();
    std::cout << "DEMO COMPLETE!" << std::endl;
    std::cout << "\nKey Features Demonstrated:" << std::endl;
    std::cout << "  ✓ MCP Server initialization" << std::endl;
    std::cout << "  ✓ Tool registration and discovery" << std::endl;
    std::cout << "  ✓ File operations (create, read, info)" << std::endl;
    std::cout << "  ✓ System command execution" << std::endl;
    std::cout << "  ✓ LLM function calling format" << std::endl;
    std::cout << "  ✓ Comprehensive error handling" << std::endl;
    std::cout << "\nReady for Phase 3: Agent System!" << std::endl;
    print_separator();
    
    return 0;
}
