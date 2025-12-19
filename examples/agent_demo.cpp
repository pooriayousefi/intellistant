#include <iostream>
#include <string>
#include "../include/agents.hpp"

using namespace pooriayousefi;

void print_separator()
{
    std::cout << "\n" << std::string(70, '=') << "\n" << std::endl;
}

void print_section(const std::string& title)
{
    print_separator();
    std::cout << "  " << title << std::endl;
    print_separator();
}

void demo_code_assistant()
{
    print_section("DEMO 1: CodeAssistant - Code Analysis");
    
    std::cout << "Creating CodeAssistant agent..." << std::endl;
    
    try
    {
        CodeAssistant assistant;
        
        std::cout << "✓ CodeAssistant created" << std::endl;
        std::cout << "  System Prompt: " << assistant.get_config().system_prompt.substr(0, 100) << "..." << std::endl;
        std::cout << "  Temperature: " << *assistant.get_config().llm_config.temperature << std::endl;
        std::cout << "  Available Tools: " << assistant.get_function_schemas().size() << std::endl;
        
        print_separator();
        std::cout << "Example Query: \"Read the file include/llm_client.hpp and analyze its structure\"" << std::endl;
        std::cout << "\nNote: With a running LLM server, the agent would:" << std::endl;
        std::cout << "  1. Call read_file tool to get the file contents" << std::endl;
        std::cout << "  2. Analyze the code structure" << std::endl;
        std::cout << "  3. Provide insights on classes, methods, and patterns" << std::endl;
        std::cout << "  4. Suggest improvements if any" << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cout << "[SKIP] CodeAssistant demo requires running llama-server" << std::endl;
        std::cout << "Error: " << e.what() << std::endl;
    }
}

void demo_devops_agent()
{
    print_section("DEMO 2: DevOpsAgent - System Management");
    
    std::cout << "Creating DevOpsAgent..." << std::endl;
    
    try
    {
        DevOpsAgent agent;
        
        std::cout << "✓ DevOpsAgent created" << std::endl;
        std::cout << "  System Prompt: " << agent.get_config().system_prompt.substr(0, 100) << "..." << std::endl;
        std::cout << "  Temperature: " << *agent.get_config().llm_config.temperature << std::endl;
        std::cout << "  Max Iterations: " << agent.get_config().max_tool_iterations << std::endl;
        
        print_separator();
        std::cout << "Example Query: \"Check the git status and list all modified files\"" << std::endl;
        std::cout << "\nNote: With a running LLM server, the agent would:" << std::endl;
        std::cout << "  1. Call git_status tool" << std::endl;
        std::cout << "  2. Parse the git output" << std::endl;
        std::cout << "  3. Present a summary of changes" << std::endl;
        std::cout << "  4. Suggest next actions (commit, push, etc.)" << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cout << "[SKIP] DevOpsAgent demo requires running llama-server" << std::endl;
    }
}

void demo_documentation_agent()
{
    print_section("DEMO 3: DocumentationAgent - Documentation Generation");
    
    std::cout << "Creating DocumentationAgent..." << std::endl;
    
    try
    {
        DocumentationAgent agent;
        
        std::cout << "✓ DocumentationAgent created" << std::endl;
        std::cout << "  System Prompt: " << agent.get_config().system_prompt.substr(0, 100) << "..." << std::endl;
        
        print_separator();
        std::cout << "Example Query: \"Create API documentation for the Agent class\"" << std::endl;
        std::cout << "\nNote: With a running LLM server, the agent would:" << std::endl;
        std::cout << "  1. Call read_file to get agent.hpp" << std::endl;
        std::cout << "  2. Analyze the Agent class interface" << std::endl;
        std::cout << "  3. Generate comprehensive API documentation" << std::endl;
        std::cout << "  4. Write documentation to a file (e.g., AGENT_API.md)" << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cout << "[SKIP] DocumentationAgent demo requires running llama-server" << std::endl;
    }
}

void demo_testing_agent()
{
    print_section("DEMO 4: TestingAgent - Test Generation");
    
    std::cout << "Creating TestingAgent..." << std::endl;
    
    try
    {
        TestingAgent agent;
        
        std::cout << "✓ TestingAgent created" << std::endl;
        std::cout << "  System Prompt: " << agent.get_config().system_prompt.substr(0, 100) << "..." << std::endl;
        
        print_separator();
        std::cout << "Example Query: \"Generate unit tests for the MCP server\"" << std::endl;
        std::cout << "\nNote: With a running LLM server, the agent would:" << std::endl;
        std::cout << "  1. Call read_file to get mcp_server.hpp" << std::endl;
        std::cout << "  2. Analyze the McpServer class methods" << std::endl;
        std::cout << "  3. Generate comprehensive unit tests" << std::endl;
        std::cout << "  4. Include edge cases and error scenarios" << std::endl;
        std::cout << "  5. Write tests to a new file" << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cout << "[SKIP] TestingAgent demo requires running llama-server" << std::endl;
    }
}

void demo_security_agent()
{
    print_section("DEMO 5: SecurityAgent - Security Analysis");
    
    std::cout << "Creating SecurityAgent..." << std::endl;
    
    try
    {
        SecurityAgent agent;
        
        std::cout << "✓ SecurityAgent created" << std::endl;
        std::cout << "  System Prompt: " << agent.get_config().system_prompt.substr(0, 100) << "..." << std::endl;
        
        print_separator();
        std::cout << "Example Query: \"Analyze the codebase for security vulnerabilities\"" << std::endl;
        std::cout << "\nNote: With a running LLM server, the agent would:" << std::endl;
        std::cout << "  1. Call search_files to find all source files" << std::endl;
        std::cout << "  2. Read and analyze each file for security issues" << std::endl;
        std::cout << "  3. Check for common vulnerabilities (OWASP Top 10)" << std::endl;
        std::cout << "  4. Generate a security report" << std::endl;
        std::cout << "  5. Recommend specific fixes" << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cout << "[SKIP] SecurityAgent demo requires running llama-server" << std::endl;
    }
}

void demo_tool_calling_workflow()
{
    print_section("DEMO 6: Tool Calling Workflow");
    
    std::cout << "This demonstrates the agent tool calling loop:" << std::endl;
    std::cout << "\n1. User sends a query to the agent" << std::endl;
    std::cout << "2. Agent sends query + available tools to LLM" << std::endl;
    std::cout << "3. LLM decides if it needs to call tools" << std::endl;
    std::cout << "4. If yes, LLM returns function calls" << std::endl;
    std::cout << "5. Agent executes each tool" << std::endl;
    std::cout << "6. Agent sends tool results back to LLM" << std::endl;
    std::cout << "7. LLM processes results and may call more tools" << std::endl;
    std::cout << "8. Loop continues until LLM provides final answer" << std::endl;
    std::cout << "9. Agent returns response to user" << std::endl;
    
    print_separator();
    std::cout << "Example Multi-Step Query:" << std::endl;
    std::cout << "\"Check git status, read the most modified file, and suggest improvements\"" << std::endl;
    
    std::cout << "\nTool Call Sequence:" << std::endl;
    std::cout << "  Iteration 1: git_status → Get modified files list" << std::endl;
    std::cout << "  Iteration 2: read_file → Read the most modified file" << std::endl;
    std::cout << "  Iteration 3: (LLM analyzes) → Generate suggestions" << std::endl;
    std::cout << "  Final: Agent returns comprehensive response" << std::endl;
}

void demo_agent_factory()
{
    print_section("DEMO 7: Agent Factory - Custom Agents");
    
    std::cout << "Creating custom agent using AgentFactory..." << std::endl;
    
    try
    {
        auto agent = AgentFactory::create_agent(
            "CustomResearchAgent",
            "You are a research assistant specialized in technical documentation. "
            "You help users find information, summarize documents, and answer technical questions. "
            "Use available tools to read files and search for information."
        );
        
        if (agent)
        {
            std::cout << "✓ Custom agent created" << std::endl;
            std::cout << "  Name: " << agent->get_config().name << std::endl;
            std::cout << "  Available Tools: " << agent->get_function_schemas().size() << std::endl;
            
            print_separator();
            std::cout << "Custom agents can be tailored for specific domains:" << std::endl;
            std::cout << "  • Legal document analysis" << std::endl;
            std::cout << "  • Medical literature research" << std::endl;
            std::cout << "  • Financial data analysis" << std::endl;
            std::cout << "  • Scientific paper summarization" << std::endl;
            std::cout << "  • Technical support automation" << std::endl;
        }
    }
    catch (const std::exception& e)
    {
        std::cout << "[SKIP] Custom agent demo requires running llama-server" << std::endl;
    }
}

void show_capabilities()
{
    print_section("AGENT SYSTEM CAPABILITIES");
    
    std::cout << "Phase 3 Delivers:" << std::endl;
    std::cout << "\n1. Base Agent System" << std::endl;
    std::cout << "   • Multi-turn conversation management" << std::endl;
    std::cout << "   • Automatic tool calling loop" << std::endl;
    std::cout << "   • Function schema generation for LLMs" << std::endl;
    std::cout << "   • Error handling and retry logic" << std::endl;
    std::cout << "   • Streaming response support" << std::endl;
    
    std::cout << "\n2. Specialized Agents (6 types)" << std::endl;
    std::cout << "   • CodeAssistant - Code analysis and generation" << std::endl;
    std::cout << "   • DevOpsAgent - Infrastructure and deployment" << std::endl;
    std::cout << "   • DocumentationAgent - Documentation generation" << std::endl;
    std::cout << "   • TestingAgent - Test generation and QA" << std::endl;
    std::cout << "   • DataAnalystAgent - Data analysis and insights" << std::endl;
    std::cout << "   • SecurityAgent - Security analysis" << std::endl;
    
    std::cout << "\n3. Tool Integration" << std::endl;
    std::cout << "   • All 12 MCP tools available to agents" << std::endl;
    std::cout << "   • Custom tool registration support" << std::endl;
    std::cout << "   • Automatic parameter validation" << std::endl;
    std::cout << "   • Result formatting for LLM context" << std::endl;
    
    std::cout << "\n4. Developer Features" << std::endl;
    std::cout << "   • AgentFactory for custom agents" << std::endl;
    std::cout << "   • Configurable LLM parameters" << std::endl;
    std::cout << "   • Conversation history management" << std::endl;
    std::cout << "   • Verbose mode for debugging" << std::endl;
}

int main()
{
    std::cout << "========================================" << std::endl;
    std::cout << "INTELLISTANT FRAMEWORK" << std::endl;
    std::cout << "Phase 3: Agent System Demo" << std::endl;
    std::cout << "========================================" << std::endl;
    
    show_capabilities();
    
    demo_code_assistant();
    demo_devops_agent();
    demo_documentation_agent();
    demo_testing_agent();
    demo_security_agent();
    demo_tool_calling_workflow();
    demo_agent_factory();
    
    print_section("CONCLUSION");
    
    std::cout << "Phase 3 Complete! ✅" << std::endl;
    std::cout << "\nThe Intellistant framework now includes:" << std::endl;
    std::cout << "  ✓ Phase 1: LLM Client with streaming" << std::endl;
    std::cout << "  ✓ Phase 2: MCP-based tool system (12 tools)" << std::endl;
    std::cout << "  ✓ Phase 3: Agent system with specialized agents" << std::endl;
    
    std::cout << "\nNext: Phase 4 - Coordinator System" << std::endl;
    std::cout << "  • Multi-agent orchestration" << std::endl;
    std::cout << "  • Intelligent request routing" << std::endl;
    std::cout << "  • Agent collaboration" << std::endl;
    std::cout << "  • Session management" << std::endl;
    
    print_separator();
    std::cout << "To run with live LLM:" << std::endl;
    std::cout << "  1. Start llama-server on port 8080" << std::endl;
    std::cout << "  2. Run: ./agent_demo" << std::endl;
    std::cout << "  3. Agents will make real tool calls!" << std::endl;
    print_separator();
    
    return 0;
}
