#include "../include/coordinator.hpp"
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>

using namespace pooriayousefi;

// ============================================================================
// CLI Helper Functions
// ============================================================================

void print_banner()
{
    std::cout << "\n";
    std::cout << "██╗███╗   ██╗████████╗███████╗██╗     ██╗     ██╗███████╗████████╗ █████╗ ███╗   ██╗████████╗\n";
    std::cout << "██║████╗  ██║╚══██╔══╝██╔════╝██║     ██║     ██║██╔════╝╚══██╔══╝██╔══██╗████╗  ██║╚══██╔══╝\n";
    std::cout << "██║██╔██╗ ██║   ██║   █████╗  ██║     ██║     ██║███████╗   ██║   ███████║██╔██╗ ██║   ██║   \n";
    std::cout << "██║██║╚██╗██║   ██║   ██╔══╝  ██║     ██║     ██║╚════██║   ██║   ██╔══██║██║╚██╗██║   ██║   \n";
    std::cout << "██║██║ ╚████║   ██║   ███████╗███████╗███████╗██║███████║   ██║   ██║  ██║██║ ╚████║   ██║   \n";
    std::cout << "╚═╝╚═╝  ╚═══╝   ╚═╝   ╚══════╝╚══════╝╚══════╝╚═╝╚══════╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝   ╚═╝   \n";
    std::cout << "\n";
    std::cout << "                    Multi-Agent Development Assistant\n";
    std::cout << "                         CLI Interface v1.0\n";
    std::cout << "\n";
}

void print_help()
{
    std::cout << "\n📚 Available Commands:\n\n";
    std::cout << "  /help              - Show this help message\n";
    std::cout << "  /agents            - List all available agents\n";
    std::cout << "  /stats             - Show agent usage statistics\n";
    std::cout << "  /session           - Show current session info\n";
    std::cout << "  /agent <name>      - Set preferred agent\n";
    std::cout << "  /routing <strategy> - Set routing strategy (intent/keyword/roundrobin)\n";
    std::cout << "  /collaborate <agents> - Multi-agent collaboration\n";
    std::cout << "  /context <key> <value> - Set session context\n";
    std::cout << "  /clear             - Clear conversation history\n";
    std::cout << "  /quit or /exit     - Exit the CLI\n";
    std::cout << "\n💬 Chat:\n";
    std::cout << "  Just type your message and press Enter to chat with agents\n";
    std::cout << "\n";
}

void print_agents(const std::vector<std::string>& agents)
{
    std::cout << "\n🤖 Available Agents (" << agents.size() << "):\n\n";
    for (const auto& agent : agents)
    {
        std::cout << "  • " << agent << "\n";
    }
    std::cout << "\n";
}

void print_stats(const std::map<std::string, int>& stats, size_t active_sessions)
{
    std::cout << "\n📊 Agent Usage Statistics:\n\n";
    
    int total = 0;
    for (const auto& [name, count] : stats)
    {
        total += count;
    }
    
    if (total == 0)
    {
        std::cout << "  No requests processed yet.\n";
    }
    else
    {
        for (const auto& [name, count] : stats)
        {
            double percentage = (count * 100.0) / total;
            std::cout << "  " << name << ": " << count << " requests (" 
                     << std::fixed << std::setprecision(1) << percentage << "%)\n";
        }
        std::cout << "\n  Total requests: " << total << "\n";
    }
    
    std::cout << "  Active sessions: " << active_sessions << "\n";
    std::cout << "\n";
}

void print_response(const CoordinatorResponse& response)
{
    std::cout << "\n╭─ Response from: " << response.agent_name;
    if (response.agents_used > 1)
    {
        std::cout << " (+" << (response.agents_used - 1) << " more agents)";
    }
    std::cout << "\n";
    
    if (!response.tool_results.empty())
    {
        std::cout << "├─ Tools used: ";
        for (size_t i = 0; i < response.tool_results.size(); ++i)
        {
            std::cout << response.tool_results[i];
            if (i < response.tool_results.size() - 1) std::cout << ", ";
        }
        std::cout << "\n";
    }
    
    std::cout << "├─ Response:\n";
    std::cout << "│\n";
    
    // Split response into lines and print with prefix
    std::istringstream iss(response.response);
    std::string line;
    while (std::getline(iss, line))
    {
        std::cout << "│  " << line << "\n";
    }
    
    std::cout << "│\n";
    
    if (response.requires_followup)
    {
        std::cout << "├─ ⚠️  Requires follow-up\n";
    }
    
    if (response.next_agent_suggestion)
    {
        std::cout << "├─ 💡 Suggested next agent: " << *response.next_agent_suggestion << "\n";
    }
    
    std::cout << "╰─\n";
}

// ============================================================================
// CLI State
// ============================================================================

struct CliState
{
    std::string session_id;
    std::string user_id{"cli_user"};
    std::optional<std::string> preferred_agent;
    RoutingStrategy routing_strategy{RoutingStrategy::IntentBased};
    std::map<std::string, std::string> context;
    int message_count{0};
};

// ============================================================================
// Command Handlers
// ============================================================================

bool handle_command(
    const std::string& input,
    Coordinator& coordinator,
    CliState& state
)
{
    std::istringstream iss(input);
    std::string command;
    iss >> command;
    
    // Convert to lowercase
    std::transform(command.begin(), command.end(), command.begin(), ::tolower);
    
    if (command == "/help" || command == "/h" || command == "/?")
    {
        print_help();
        return true;
    }
    
    if (command == "/quit" || command == "/exit" || command == "/q")
    {
        std::cout << "\n👋 Goodbye!\n\n";
        return false;
    }
    
    if (command == "/agents")
    {
        auto agents = coordinator.list_agents();
        print_agents(agents);
        return true;
    }
    
    if (command == "/stats")
    {
        auto stats = coordinator.get_agent_usage_stats();
        auto sessions = coordinator.get_active_sessions_count();
        print_stats(stats, sessions);
        return true;
    }
    
    if (command == "/session")
    {
        std::cout << "\n📋 Current Session:\n\n";
        std::cout << "  Session ID: " << state.session_id << "\n";
        std::cout << "  User ID: " << state.user_id << "\n";
        std::cout << "  Messages: " << state.message_count << "\n";
        std::cout << "  Routing: ";
        switch (state.routing_strategy)
        {
            case RoutingStrategy::IntentBased: std::cout << "Intent-Based"; break;
            case RoutingStrategy::KeywordBased: std::cout << "Keyword-Based"; break;
            case RoutingStrategy::RoundRobin: std::cout << "Round-Robin"; break;
            case RoutingStrategy::PreferredAgent: std::cout << "Preferred Agent"; break;
        }
        std::cout << "\n";
        
        if (state.preferred_agent)
        {
            std::cout << "  Preferred Agent: " << *state.preferred_agent << "\n";
        }
        
        if (!state.context.empty())
        {
            std::cout << "  Context:\n";
            for (const auto& [key, value] : state.context)
            {
                std::cout << "    " << key << " = " << value << "\n";
            }
        }
        
        std::cout << "\n";
        return true;
    }
    
    if (command == "/agent")
    {
        std::string agent_name;
        iss >> agent_name;
        
        if (agent_name.empty())
        {
            state.preferred_agent = std::nullopt;
            std::cout << "\n✓ Cleared preferred agent\n\n";
        }
        else
        {
            state.preferred_agent = agent_name;
            std::cout << "\n✓ Set preferred agent to: " << agent_name << "\n\n";
        }
        return true;
    }
    
    if (command == "/routing")
    {
        std::string strategy;
        iss >> strategy;
        
        std::transform(strategy.begin(), strategy.end(), strategy.begin(), ::tolower);
        
        if (strategy == "intent")
        {
            state.routing_strategy = RoutingStrategy::IntentBased;
            std::cout << "\n✓ Set routing to Intent-Based\n\n";
        }
        else if (strategy == "keyword")
        {
            state.routing_strategy = RoutingStrategy::KeywordBased;
            std::cout << "\n✓ Set routing to Keyword-Based\n\n";
        }
        else if (strategy == "roundrobin")
        {
            state.routing_strategy = RoutingStrategy::RoundRobin;
            std::cout << "\n✓ Set routing to Round-Robin\n\n";
        }
        else
        {
            std::cout << "\n❌ Unknown routing strategy. Use: intent, keyword, or roundrobin\n\n";
        }
        return true;
    }
    
    if (command == "/context")
    {
        std::string key, value;
        iss >> key;
        std::getline(iss, value);
        
        // Trim leading spaces
        value.erase(0, value.find_first_not_of(" \t"));
        
        if (key.empty() || value.empty())
        {
            std::cout << "\n❌ Usage: /context <key> <value>\n\n";
        }
        else
        {
            state.context[key] = value;
            coordinator.update_session_context(state.session_id, key, value);
            std::cout << "\n✓ Set context: " << key << " = " << value << "\n\n";
        }
        return true;
    }
    
    if (command == "/clear")
    {
        // End current session and create new one
        coordinator.end_session(state.session_id);
        
        state.session_id = "cli_session_" + std::to_string(
            std::chrono::system_clock::now().time_since_epoch().count()
        );
        
        coordinator.create_session(state.session_id, state.user_id);
        state.message_count = 0;
        
        std::cout << "\n✓ Cleared conversation history\n\n";
        return true;
    }
    
    if (command == "/collaborate")
    {
        std::string rest;
        std::getline(iss, rest);
        rest.erase(0, rest.find_first_not_of(" \t"));
        
        if (rest.empty())
        {
            std::cout << "\n❌ Usage: /collaborate <task> | <agent1>,<agent2>,...\n\n";
            return true;
        }
        
        auto delimiter_pos = rest.find('|');
        if (delimiter_pos == std::string::npos)
        {
            std::cout << "\n❌ Usage: /collaborate <task> | <agent1>,<agent2>,...\n\n";
            return true;
        }
        
        std::string task = rest.substr(0, delimiter_pos);
        std::string agents_str = rest.substr(delimiter_pos + 1);
        
        // Trim
        task.erase(0, task.find_first_not_of(" \t"));
        task.erase(task.find_last_not_of(" \t") + 1);
        agents_str.erase(0, agents_str.find_first_not_of(" \t"));
        
        // Parse agent names
        std::vector<std::string> agent_names;
        std::istringstream agent_stream(agents_str);
        std::string agent;
        while (std::getline(agent_stream, agent, ','))
        {
            agent.erase(0, agent.find_first_not_of(" \t"));
            agent.erase(agent.find_last_not_of(" \t") + 1);
            if (!agent.empty())
            {
                agent_names.push_back(agent);
            }
        }
        
        if (agent_names.empty())
        {
            std::cout << "\n❌ No agents specified\n\n";
            return true;
        }
        
        std::cout << "\n🤝 Collaborating with " << agent_names.size() << " agents...\n";
        
        auto result = coordinator.collaborate(task, agent_names);
        if (result)
        {
            print_response(*result);
        }
        else
        {
            std::cout << "\n❌ Error: " << result.error() << "\n\n";
        }
        
        return true;
    }
    
    std::cout << "\n❌ Unknown command: " << command << "\n";
    std::cout << "Type /help for available commands\n\n";
    return true;
}

// ============================================================================
// Main CLI Loop
// ============================================================================

int main(int argc, char* argv[])
{
    // Parse command line arguments
    std::string llm_server_url = "localhost:8080";
    
    for (int i = 1; i < argc; i++)
    {
        std::string arg = argv[i];
        if (arg == "--llm-server" && i + 1 < argc)
        {
            llm_server_url = argv[++i];
        }
        else if (arg == "--help" || arg == "-h")
        {
            std::cout << "Usage: " << argv[0] << " [options]\n";
            std::cout << "Options:\n";
            std::cout << "  --llm-server <url>  LLM server URL (default: localhost:8080)\n";
            std::cout << "  --help, -h          Show this help\n";
            return 0;
        }
    }
    
    print_banner();
    
    std::cout << "🔧 Initializing coordinator...\n";
    
    try
    {
        Coordinator coordinator(llm_server_url, RoutingStrategy::IntentBased, true);
        
        CliState state;
        state.session_id = "cli_session_" + std::to_string(
            std::chrono::system_clock::now().time_since_epoch().count()
        );
        
        coordinator.create_session(state.session_id, state.user_id);
        
        auto agents = coordinator.list_agents();
        std::cout << "✓ Loaded " << agents.size() << " specialized agents\n";
        std::cout << "✓ Connected to LLM server: " << llm_server_url << "\n\n";
        
        std::cout << "Type /help for available commands, or just chat with the agents!\n";
        std::cout << std::string(80, '─') << "\n\n";
        
        // Main loop
        while (true)
        {
            std::cout << "You> " << std::flush;
            
            std::string input;
            if (!std::getline(std::cin, input))
            {
                break;  // EOF or error
            }
            
            // Trim whitespace
            input.erase(0, input.find_first_not_of(" \t\n\r"));
            input.erase(input.find_last_not_of(" \t\n\r") + 1);
            
            if (input.empty())
            {
                continue;
            }
            
            // Handle commands
            if (input[0] == '/')
            {
                if (!handle_command(input, coordinator, state))
                {
                    break;  // Quit command
                }
                continue;
            }
            
            // Send message to coordinator
            UserRequest request;
            request.user_id = state.user_id;
            request.session_id = state.session_id;
            request.message = input;
            request.preferred_agent = state.preferred_agent;
            request.metadata = state.context;
            
            auto result = coordinator.handle_request(request);
            
            if (result)
            {
                print_response(*result);
                state.message_count++;
            }
            else
            {
                std::cout << "\n❌ Error: " << result.error() << "\n\n";
            }
        }
        
        coordinator.end_session(state.session_id);
    }
    catch (const std::exception& e)
    {
        std::cerr << "\n❌ Fatal error: " << e.what() << "\n\n";
        return 1;
    }
    
    return 0;
}
