#include "../include/coordinator.hpp"
#include <iostream>
#include <thread>
#include <chrono>

using namespace pooriayousefi;

// Demo helper
void print_section(const std::string& title)
{
    std::cout << "\n" << std::string(70, '=') << std::endl;
    std::cout << title << std::endl;
    std::cout << std::string(70, '=') << std::endl;
}

void print_response(const CoordinatorResponse& response)
{
    std::cout << "\n┌─ Response from: " << response.agent_name << std::endl;
    std::cout << "├─ Agents used: " << response.agents_used << std::endl;
    std::cout << "├─ Tools called: " << response.tool_results.size() << std::endl;
    
    if (!response.tool_results.empty())
    {
        std::cout << "│  Tools: ";
        for (size_t i = 0; i < response.tool_results.size(); ++i)
        {
            std::cout << response.tool_results[i];
            if (i < response.tool_results.size() - 1) std::cout << ", ";
        }
        std::cout << std::endl;
    }
    
    std::cout << "├─ Requires followup: " << (response.requires_followup ? "Yes" : "No") << std::endl;
    
    if (response.next_agent_suggestion)
    {
        std::cout << "├─ Suggested next agent: " << *response.next_agent_suggestion << std::endl;
    }
    
    std::cout << "└─ Response:" << std::endl;
    std::cout << response.response << std::endl;
}

// ============================================================================
// Demo 1: Basic Request Routing
// ============================================================================

void demo_basic_routing()
{
    print_section("DEMO 1: Basic Request Routing");
    
    std::cout << "\nCreating coordinator with keyword-based routing..." << std::endl;
    Coordinator coordinator("localhost:8080", RoutingStrategy::KeywordBased, true);
    
    std::cout << "\nRegistered agents:" << std::endl;
    auto agents = coordinator.list_agents();
    for (const auto& agent_name : agents)
    {
        std::cout << "  • " << agent_name << std::endl;
    }
    
    // Example requests
    std::vector<std::string> sample_requests = {
        "Can you review the authentication code for potential bugs?",
        "Deploy the updated service to the staging environment",
        "Generate API documentation for the user management module",
        "Run the test suite and check coverage",
        "Analyze the performance metrics from yesterday"
    };
    
    std::cout << "\n" << std::string(70, '-') << std::endl;
    std::cout << "Routing sample requests (structure demonstration):" << std::endl;
    std::cout << std::string(70, '-') << std::endl;
    
    for (const auto& msg : sample_requests)
    {
        std::cout << "\n📨 Request: \"" << msg << "\"" << std::endl;
        std::cout << "   (Keyword-based routing will analyze and route appropriately)" << std::endl;
    }
}

// ============================================================================
// Demo 2: Session Management
// ============================================================================

void demo_session_management()
{
    print_section("DEMO 2: Session Management");
    
    Coordinator coordinator("localhost:8080");
    
    std::cout << "\nCreating user session..." << std::endl;
    coordinator.create_session("session_001", "developer_alice");
    
    std::cout << "Setting session context..." << std::endl;
    coordinator.update_session_context("session_001", "project", "intellistant");
    coordinator.update_session_context("session_001", "language", "C++23");
    coordinator.update_session_context("session_001", "focus", "agent_system");
    
    auto session = coordinator.get_session("session_001");
    if (session)
    {
        std::cout << "\n📋 Session Details:" << std::endl;
        std::cout << "   Session ID: " << session->session_id << std::endl;
        std::cout << "   User ID: " << session->user_id << std::endl;
        std::cout << "   Context:" << std::endl;
        
        for (const auto& [key, value] : session->context)
        {
            std::cout << "      " << key << " = " << value << std::endl;
        }
        
        auto now = std::chrono::system_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(
            now - session->created_at
        );
        
        std::cout << "   Age: " << duration.count() << " seconds" << std::endl;
    }
    
    std::cout << "\nActive sessions: " << coordinator.get_active_sessions_count() << std::endl;
}

// ============================================================================
// Demo 3: Multi-Agent Collaboration
// ============================================================================

void demo_collaboration()
{
    print_section("DEMO 3: Multi-Agent Collaboration");
    
    Coordinator coordinator("localhost:8080", RoutingStrategy::KeywordBased, true);
    
    std::cout << "\nScenario: Preparing a module for production release" << std::endl;
    std::cout << "This requires multiple specialized agents working together:" << std::endl;
    
    std::vector<std::string> collab_agents = {
        "CodeAssistant",   // Review code quality
        "TestingAgent",    // Verify test coverage
        "SecurityAgent",   // Security audit
        "DocumentationAgent"  // Ensure docs are complete
    };
    
    std::cout << "\n🤝 Collaborating agents:" << std::endl;
    for (const auto& agent : collab_agents)
    {
        std::cout << "   • " << agent << std::endl;
    }
    
    std::string task = "Review the authentication module for production readiness: "
                      "check code quality, test coverage, security vulnerabilities, "
                      "and documentation completeness.";
    
    std::cout << "\n📝 Task: " << task << std::endl;
    std::cout << "\n⏳ Each agent would contribute their specialized analysis..." << std::endl;
    std::cout << "   (Skipping actual LLM calls in demo)" << std::endl;
    
    // Simulate collaboration structure
    std::cout << "\n✅ Collaboration workflow:" << std::endl;
    std::cout << "   1. CodeAssistant reviews code structure and style" << std::endl;
    std::cout << "   2. TestingAgent checks unit test coverage" << std::endl;
    std::cout << "   3. SecurityAgent performs security audit" << std::endl;
    std::cout << "   4. DocumentationAgent verifies API documentation" << std::endl;
    std::cout << "   5. Coordinator aggregates all findings" << std::endl;
}

// ============================================================================
// Demo 4: Different Routing Strategies
// ============================================================================

void demo_routing_strategies()
{
    print_section("DEMO 4: Routing Strategy Comparison");
    
    std::cout << "\nCreating coordinators with different routing strategies..." << std::endl;
    
    // Keyword-based
    std::cout << "\n1️⃣  KEYWORD-BASED ROUTING" << std::endl;
    std::cout << "   • Fast pattern matching" << std::endl;
    std::cout << "   • Searches for domain-specific keywords" << std::endl;
    std::cout << "   • No LLM call required" << std::endl;
    std::cout << "   Example: 'deploy' → DevOpsAgent, 'bug' → CodeAssistant" << std::endl;
    
    Coordinator keyword_coord("localhost:8080", RoutingStrategy::KeywordBased, false);
    
    // Intent-based
    std::cout << "\n2️⃣  INTENT-BASED ROUTING" << std::endl;
    std::cout << "   • Uses LLM to analyze user intent" << std::endl;
    std::cout << "   • More accurate for complex requests" << std::endl;
    std::cout << "   • Understands context and nuance" << std::endl;
    std::cout << "   Example: 'Make sure the API is secure' → SecurityAgent" << std::endl;
    
    Coordinator intent_coord("localhost:8080", RoutingStrategy::IntentBased, false);
    
    // Round-robin
    std::cout << "\n3️⃣  ROUND-ROBIN ROUTING" << std::endl;
    std::cout << "   • Distributes load evenly across agents" << std::endl;
    std::cout << "   • No analysis required" << std::endl;
    std::cout << "   • Good for load balancing" << std::endl;
    
    Coordinator rr_coord("localhost:8080", RoutingStrategy::RoundRobin, false);
    
    // Preferred agent
    std::cout << "\n4️⃣  PREFERRED AGENT" << std::endl;
    std::cout << "   • User explicitly chooses agent" << std::endl;
    std::cout << "   • Falls back to other strategies if invalid" << std::endl;
    std::cout << "   • Gives users direct control" << std::endl;
    
    std::cout << "\n💡 Recommendation: Use Intent-Based for best user experience" << std::endl;
    std::cout << "               Use Keyword-Based for faster responses" << std::endl;
}

// ============================================================================
// Demo 5: Agent Usage Statistics
// ============================================================================

void demo_statistics()
{
    print_section("DEMO 5: Agent Usage Statistics");
    
    Coordinator coordinator("localhost:8080", RoutingStrategy::KeywordBased, false);
    
    std::cout << "\nInitial agent usage statistics:" << std::endl;
    auto stats = coordinator.get_agent_usage_stats();
    
    std::cout << "\n┌─ Agent Statistics" << std::endl;
    for (const auto& [agent_name, count] : stats)
    {
        std::cout << "├─ " << agent_name << ": " << count << " requests" << std::endl;
    }
    std::cout << "└─ Total tracked agents: " << stats.size() << std::endl;
    
    std::cout << "\n📊 After processing requests, these stats would show:" << std::endl;
    std::cout << "   • Which agents are most frequently used" << std::endl;
    std::cout << "   • Load distribution across agents" << std::endl;
    std::cout << "   • Helps identify bottlenecks" << std::endl;
    std::cout << "   • Can inform scaling decisions" << std::endl;
}

// ============================================================================
// Demo 6: Real-World Scenarios
// ============================================================================

void demo_real_world_scenarios()
{
    print_section("DEMO 6: Real-World Usage Scenarios");
    
    Coordinator coordinator("localhost:8080", RoutingStrategy::IntentBased, false);
    
    std::cout << "\n📚 Scenario Library:\n" << std::endl;
    
    // Scenario 1
    std::cout << "🎯 Scenario 1: Bug Investigation\n";
    std::cout << "   Developer: \"Users are reporting login failures\"" << std::endl;
    std::cout << "   → Routes to: DevOpsAgent (checks logs)" << std::endl;
    std::cout << "   → Then: SecurityAgent (auth verification)" << std::endl;
    std::cout << "   → Then: CodeAssistant (code review)" << std::endl;
    
    // Scenario 2
    std::cout << "\n🎯 Scenario 2: New Feature Development\n";
    std::cout << "   Developer: \"Add OAuth2 support to the API\"" << std::endl;
    std::cout << "   → Routes to: SecurityAgent (security design)" << std::endl;
    std::cout << "   → Then: CodeAssistant (implementation)" << std::endl;
    std::cout << "   → Then: TestingAgent (test creation)" << std::endl;
    std::cout << "   → Finally: DocumentationAgent (API docs)" << std::endl;
    
    // Scenario 3
    std::cout << "\n🎯 Scenario 3: Performance Issue\n";
    std::cout << "   Developer: \"The dashboard is loading slowly\"" << std::endl;
    std::cout << "   → Routes to: DataAnalystAgent (analyze metrics)" << std::endl;
    std::cout << "   → Then: CodeAssistant (find bottlenecks)" << std::endl;
    std::cout << "   → Then: DevOpsAgent (infrastructure check)" << std::endl;
    
    // Scenario 4
    std::cout << "\n🎯 Scenario 4: Production Deployment\n";
    std::cout << "   Developer: \"Ready to deploy v2.0 to production\"" << std::endl;
    std::cout << "   → Collaboration:" << std::endl;
    std::cout << "      • TestingAgent: Verify all tests pass" << std::endl;
    std::cout << "      • SecurityAgent: Security checklist" << std::endl;
    std::cout << "      • DocumentationAgent: Release notes" << std::endl;
    std::cout << "      • DevOpsAgent: Execute deployment" << std::endl;
}

// ============================================================================
// Demo 7: Custom Agent Registration
// ============================================================================

void demo_custom_agent()
{
    print_section("DEMO 7: Custom Agent Registration");
    
    Coordinator coordinator("localhost:8080");
    
    std::cout << "\nCreating custom specialized agent..." << std::endl;
    
    AgentConfig config;
    config.name = "DatabaseAgent";
    config.system_prompt = 
        "You are a database optimization specialist. "
        "You help with query optimization, schema design, "
        "index management, and database performance tuning.";
    config.llm_config.temperature = 0.2;
    config.llm_config.max_tokens = 800;
    
    auto db_agent = std::make_shared<Agent>(config, "localhost:8080");
    
    std::cout << "Registering DatabaseAgent..." << std::endl;
    coordinator.register_agent("DatabaseAgent", db_agent);
    
    auto agents = coordinator.list_agents();
    std::cout << "\nTotal agents now: " << agents.size() << std::endl;
    std::cout << "\nAll registered agents:" << std::endl;
    for (const auto& agent_name : agents)
    {
        std::cout << "   • " << agent_name << std::endl;
    }
    
    std::cout << "\n💡 Custom agents enable domain-specific expertise!" << std::endl;
    std::cout << "   You can add agents for:" << std::endl;
    std::cout << "   • Database optimization" << std::endl;
    std::cout << "   • Machine learning workflows" << std::endl;
    std::cout << "   • Frontend development" << std::endl;
    std::cout << "   • Mobile app development" << std::endl;
    std::cout << "   • Any specialized domain!" << std::endl;
}

// ============================================================================
// Main Demo
// ============================================================================

int main()
{
    std::cout << "\n";
    std::cout << "██╗███╗   ██╗████████╗███████╗██╗     ██╗     ██╗███████╗████████╗ █████╗ ███╗   ██╗████████╗\n";
    std::cout << "██║████╗  ██║╚══██╔══╝██╔════╝██║     ██║     ██║██╔════╝╚══██╔══╝██╔══██╗████╗  ██║╚══██╔══╝\n";
    std::cout << "██║██╔██╗ ██║   ██║   █████╗  ██║     ██║     ██║███████╗   ██║   ███████║██╔██╗ ██║   ██║   \n";
    std::cout << "██║██║╚██╗██║   ██║   ██╔══╝  ██║     ██║     ██║╚════██║   ██║   ██╔══██║██║╚██╗██║   ██║   \n";
    std::cout << "██║██║ ╚████║   ██║   ███████╗███████╗███████╗██║███████║   ██║   ██║  ██║██║ ╚████║   ██║   \n";
    std::cout << "╚═╝╚═╝  ╚═══╝   ╚═╝   ╚══════╝╚══════╝╚══════╝╚═╝╚══════╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝   ╚═╝   \n";
    std::cout << "\n";
    std::cout << "                    COORDINATOR SYSTEM DEMONSTRATION\n";
    std::cout << "                         Phase 4 - Complete\n";
    std::cout << "\n";
    
    try
    {
        demo_basic_routing();
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
        demo_session_management();
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
        demo_collaboration();
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
        demo_routing_strategies();
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
        demo_statistics();
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
        demo_real_world_scenarios();
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
        demo_custom_agent();
        
        print_section("DEMO COMPLETE");
        std::cout << "\n✨ The Coordinator System enables:" << std::endl;
        std::cout << "   ✓ Intelligent request routing to specialized agents" << std::endl;
        std::cout << "   ✓ Multi-agent collaboration for complex tasks" << std::endl;
        std::cout << "   ✓ Session management with context tracking" << std::endl;
        std::cout << "   ✓ Multiple routing strategies" << std::endl;
        std::cout << "   ✓ Custom agent registration" << std::endl;
        std::cout << "   ✓ Usage statistics and monitoring" << std::endl;
        
        std::cout << "\n🎯 Ready for production use!" << std::endl;
        std::cout << "\n📚 For full documentation, see docs/PHASE4_COMPLETE.md" << std::endl;
        std::cout << "\n" << std::string(70, '=') << std::endl;
        
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "\n❌ Demo failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
