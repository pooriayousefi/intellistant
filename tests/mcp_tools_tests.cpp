#include <iostream>
#include <cassert>
#include <fstream>
#include <filesystem>
#include "../include/mcp_tools.hpp"

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

void test_mcp_server_initialization()
{
    test_header("MCP Server Initialization");
    
    auto server = std::make_shared<McpServer>("TestServer", "1.0.0");
    auto client = std::make_shared<McpClient>(server);
    
    // Initialize client
    auto result = client->initialize("TestClient", "1.0.0");
    
    if (!result)
    {
        test_failure("Failed to initialize: " + result.error().message);
    }
    
    test_success("Server initialized");
    std::cout << "Server: " << result->server_info.name << " v" << result->server_info.version << std::endl;
    std::cout << "Protocol: " << result->protocol_version << std::endl;
}

void test_tool_registration()
{
    test_header("Tool Registration");
    
    auto server = std::make_shared<McpServer>("TestServer", "1.0.0");
    auto client = std::make_shared<McpClient>(server);
    
    // Initialize
    client->initialize("TestClient", "1.0.0");
    
    // Register file system tools
    ToolRegistry::register_filesystem_tools(*server);
    
    // List tools
    auto tools = client->list_tools();
    
    if (!tools)
    {
        test_failure("Failed to list tools: " + tools.error().message);
    }
    
    test_success("Tools registered successfully");
    std::cout << "Number of tools: " << tools->size() << std::endl;
    
    for (const auto& tool : *tools)
    {
        std::cout << "  - " << tool.name << ": " << tool.description << std::endl;
    }
}

void test_file_operations()
{
    test_header("File Operations");
    
    auto server = std::make_shared<McpServer>("TestServer", "1.0.0");
    auto client = std::make_shared<McpClient>(server);
    
    client->initialize("TestClient", "1.0.0");
    ToolRegistry::register_filesystem_tools(*server);
    
    std::string test_dir = "/tmp/mcp_test";
    std::string test_file = test_dir + "/test_file.txt";
    std::string test_content = "Hello, MCP Tools!";
    
    // Clean up first
    std::filesystem::remove_all(test_dir);
    
    // Create directory
    {
        nlohmann::json args;
        args["path"] = test_dir;
        
        auto result = client->call_tool("create_directory", args);
        
        if (!result)
        {
            test_failure("Failed to create directory: " + result.error().message);
        }
        
        test_success("Directory created");
    }
    
    // Write file
    {
        nlohmann::json args;
        args["path"] = test_file;
        args["content"] = test_content;
        
        auto result = client->call_tool("write_file", args);
        
        if (!result)
        {
            test_failure("Failed to write file: " + result.error().message);
        }
        
        test_success("File written");
    }
    
    // Read file
    {
        nlohmann::json args;
        args["path"] = test_file;
        
        auto result = client->call_tool("read_file", args);
        
        if (!result)
        {
            test_failure("Failed to read file: " + result.error().message);
        }
        
        if (!result->content.empty())
        {
            std::string content = result->content[0]["text"];
            if (content == test_content)
            {
                test_success("File read correctly");
            }
            else
            {
                test_failure("File content mismatch");
            }
        }
    }
    
    // File info
    {
        nlohmann::json args;
        args["path"] = test_file;
        
        auto result = client->call_tool("file_info", args);
        
        if (!result)
        {
            test_failure("Failed to get file info: " + result.error().message);
        }
        
        test_success("File info retrieved");
    }
    
    // List directory
    {
        nlohmann::json args;
        args["path"] = test_dir;
        
        auto result = client->call_tool("list_directory", args);
        
        if (!result)
        {
            test_failure("Failed to list directory: " + result.error().message);
        }
        
        test_success("Directory listed");
    }
    
    // Clean up
    std::filesystem::remove_all(test_dir);
}

void test_search_files()
{
    test_header("File Search");
    
    auto server = std::make_shared<McpServer>("TestServer", "1.0.0");
    auto client = std::make_shared<McpClient>(server);
    
    client->initialize("TestClient", "1.0.0");
    ToolRegistry::register_filesystem_tools(*server);
    
    std::string test_dir = "/tmp/mcp_search_test";
    std::filesystem::remove_all(test_dir);
    std::filesystem::create_directories(test_dir);
    
    // Create test files
    std::ofstream(test_dir + "/test1.cpp") << "// C++ file";
    std::ofstream(test_dir + "/test2.hpp") << "// Header file";
    std::ofstream(test_dir + "/readme.md") << "# Readme";
    
    // Search for .cpp files
    {
        nlohmann::json args;
        args["path"] = test_dir;
        args["pattern"] = "\\.cpp$";
        
        auto result = client->call_tool("search_files", args);
        
        if (!result)
        {
            test_failure("Failed to search files: " + result.error().message);
        }
        
        test_success("Files searched");
    }
    
    // Clean up
    std::filesystem::remove_all(test_dir);
}

void test_git_operations()
{
    test_header("Git Operations");
    
    auto server = std::make_shared<McpServer>("TestServer", "1.0.0");
    auto client = std::make_shared<McpClient>(server);
    
    client->initialize("TestClient", "1.0.0");
    ToolRegistry::register_git_tools(*server);
    
    // Use current project directory (absolute path)
    std::string repo_path = "/home/pooria-yousefi/github.com/pooriayousefi/intellistant";
    
    // Git status
    {
        nlohmann::json args;
        args["repo_path"] = repo_path;
        
        auto result = client->call_tool("git_status", args);
        
        if (!result)
        {
            std::cout << "[SKIP] Git not available or not a git repo" << std::endl;
            return;
        }
        
        test_success("Git status retrieved");
    }
    
    // Git log
    {
        nlohmann::json args;
        args["repo_path"] = repo_path;
        args["limit"] = 5;
        
        auto result = client->call_tool("git_log", args);
        
        if (!result)
        {
            test_failure("Failed to get git log: " + result.error().message);
        }
        
        test_success("Git log retrieved");
    }
    
    // Git branch list
    {
        nlohmann::json args;
        args["repo_path"] = repo_path;
        
        auto result = client->call_tool("git_branch_list", args);
        
        if (!result)
        {
            test_failure("Failed to list branches: " + result.error().message);
        }
        
        test_success("Git branches listed");
    }
}

void test_error_handling()
{
    test_header("Error Handling");
    
    auto server = std::make_shared<McpServer>("TestServer", "1.0.0");
    auto client = std::make_shared<McpClient>(server);
    
    client->initialize("TestClient", "1.0.0");
    ToolRegistry::register_filesystem_tools(*server);
    
    // Try to read non-existent file
    {
        nlohmann::json args;
        args["path"] = "/nonexistent/file.txt";
        
        auto result = client->call_tool("read_file", args);
        
        if (result)
        {
            test_failure("Should have failed for non-existent file");
        }
        
        test_success("Non-existent file error handled correctly");
    }
    
    // Try to call non-existent tool
    {
        nlohmann::json args;
        
        auto result = client->call_tool("nonexistent_tool", args);
        
        if (result)
        {
            test_failure("Should have failed for non-existent tool");
        }
        
        test_success("Non-existent tool error handled correctly");
    }
    
    // Try to call tool with missing parameters
    {
        nlohmann::json args;  // Missing required "path" parameter
        
        auto result = client->call_tool("read_file", args);
        
        if (result)
        {
            test_failure("Should have failed for missing parameters");
        }
        
        test_success("Missing parameter error handled correctly");
    }
}

void test_json_rpc_protocol()
{
    test_header("JSON-RPC 2.0 Protocol Compliance");
    
    auto server = std::make_shared<McpServer>("TestServer", "1.0.0");
    
    // Test valid request
    {
        nlohmann::json request;
        request["jsonrpc"] = "2.0";
        request["method"] = "initialize";
        request["id"] = 1;
        request["params"] = nlohmann::json{
            {"protocolVersion", "2024-11-05"},
            {"clientInfo", {{"name", "Test"}, {"version", "1.0"}}},
            {"capabilities", {{"tools", nlohmann::json::object()}}}
        };
        
        auto response = server->handle_request(request);
        
        if (!response.contains("result"))
        {
            test_failure("Valid request should return result");
        }
        
        test_success("Valid JSON-RPC request handled");
    }
    
    // Test invalid JSON-RPC version
    {
        nlohmann::json request;
        request["jsonrpc"] = "1.0";  // Invalid version
        request["method"] = "initialize";
        request["id"] = 2;
        
        auto response = server->handle_request(request);
        
        if (!response.contains("error"))
        {
            test_failure("Invalid version should return error");
        }
        
        test_success("Invalid JSON-RPC version rejected");
    }
    
    // Test missing method
    {
        nlohmann::json request;
        request["jsonrpc"] = "2.0";
        request["id"] = 3;
        // Missing "method"
        
        auto response = server->handle_request(request);
        
        if (!response.contains("error"))
        {
            test_failure("Missing method should return error");
        }
        
        test_success("Missing method rejected");
    }
}

void test_function_calling_format()
{
    test_header("LLM Function Calling Format");
    
    auto server = std::make_shared<McpServer>("TestServer", "1.0.0");
    ToolRegistry::register_filesystem_tools(*server);
    
    auto schemas = server->get_function_schemas();
    
    if (schemas.empty())
    {
        test_failure("Should have function schemas");
    }
    
    test_success("Function schemas generated");
    std::cout << "Number of function schemas: " << schemas.size() << std::endl;
    
    // Verify schema format
    for (const auto& schema : schemas)
    {
        if (!schema.contains("type") || schema["type"] != "function")
        {
            test_failure("Invalid schema format");
        }
        
        if (!schema.contains("function"))
        {
            test_failure("Missing function definition");
        }
        
        const auto& func = schema["function"];
        if (!func.contains("name") || !func.contains("description") || !func.contains("parameters"))
        {
            test_failure("Incomplete function definition");
        }
    }
    
    test_success("Function calling format valid");
    
    // Print first schema as example
    if (!schemas.empty())
    {
        std::cout << "Example schema:\n" << schemas[0].dump(2) << std::endl;
    }
}

void test_execute_command()
{
    test_header("Execute Command");
    
    auto server = std::make_shared<McpServer>("TestServer", "1.0.0");
    auto client = std::make_shared<McpClient>(server);
    
    client->initialize("TestClient", "1.0.0");
    ToolRegistry::register_system_tools(*server);
    
    // Execute simple command
    {
        nlohmann::json args;
        args["command"] = "echo 'Hello from MCP'";
        
        auto result = client->call_tool("execute_command", args);
        
        if (!result)
        {
            test_failure("Failed to execute command: " + result.error().message);
        }
        
        test_success("Command executed");
    }
}

// ============================================================================
// Main Test Runner
// ============================================================================

int main()
{
    std::cout << "========================================" << std::endl;
    std::cout << "MCP TOOLS TEST SUITE" << std::endl;
    std::cout << "========================================" << std::endl;
    
    try
    {
        test_mcp_server_initialization();
        test_tool_registration();
        test_file_operations();
        test_search_files();
        test_git_operations();
        test_error_handling();
        test_json_rpc_protocol();
        test_function_calling_format();
        test_execute_command();
        
        std::cout << "\n========================================" << std::endl;
        std::cout << "ALL TESTS PASSED! ✓" << std::endl;
        std::cout << "========================================" << std::endl;
        
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
