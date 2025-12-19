#include "../include/api_server.hpp"
#include <iostream>
#include <csignal>
#include <memory>

using namespace pooriayousefi;

std::unique_ptr<ApiServer> server_instance;

void signal_handler(int signal)
{
    std::cout << "\n\n🛑 Shutting down server...\n";
    if (server_instance)
    {
        server_instance->stop();
    }
    exit(0);
}

int main(int argc, char* argv[])
{
    // Parse command line arguments
    ApiServerConfig config;
    
    for (int i = 1; i < argc; i++)
    {
        std::string arg = argv[i];
        if (arg == "--port" && i + 1 < argc)
        {
            config.port = std::stoi(argv[++i]);
        }
        else if (arg == "--host" && i + 1 < argc)
        {
            config.host = argv[++i];
        }
        else if (arg == "--llm-server" && i + 1 < argc)
        {
            config.llm_server_url = argv[++i];
        }
        else if (arg == "--routing" && i + 1 < argc)
        {
            std::string strategy = argv[++i];
            if (strategy == "intent")
            {
                config.default_routing = RoutingStrategy::IntentBased;
            }
            else if (strategy == "keyword")
            {
                config.default_routing = RoutingStrategy::KeywordBased;
            }
            else if (strategy == "roundrobin")
            {
                config.default_routing = RoutingStrategy::RoundRobin;
            }
            else
            {
                std::cerr << "Unknown routing strategy: " << strategy << std::endl;
                return 1;
            }
        }
        else if (arg == "--help" || arg == "-h")
        {
            std::cout << "Usage: " << argv[0] << " [options]\n";
            std::cout << "Options:\n";
            std::cout << "  --port <port>       Server port (default: 8000)\n";
            std::cout << "  --host <host>       Server host (default: 0.0.0.0)\n";
            std::cout << "  --llm-server <url>  LLM server URL (default: localhost:8080)\n";
            std::cout << "  --routing <strategy> Routing strategy: intent/keyword/roundrobin (default: intent)\n";
            std::cout << "  --help, -h          Show this help\n";
            return 0;
        }
    }
    
    // Setup signal handlers
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);
    
    std::cout << "\n";
    std::cout << "██╗███╗   ██╗████████╗███████╗██╗     ██╗     ██╗███████╗████████╗ █████╗ ███╗   ██╗████████╗\n";
    std::cout << "██║████╗  ██║╚══██╔══╝██╔════╝██║     ██║     ██║██╔════╝╚══██╔══╝██╔══██╗████╗  ██║╚══██╔══╝\n";
    std::cout << "██║██╔██╗ ██║   ██║   █████╗  ██║     ██║     ██║███████╗   ██║   ███████║██╔██╗ ██║   ██║   \n";
    std::cout << "██║██║╚██╗██║   ██║   ██╔══╝  ██║     ██║     ██║╚════██║   ██║   ██╔══██║██║╚██╗██║   ██║   \n";
    std::cout << "██║██║ ╚████║   ██║   ███████╗███████╗███████╗██║███████║   ██║   ██║  ██║██║ ╚████║   ██║   \n";
    std::cout << "╚═╝╚═╝  ╚═══╝   ╚═╝   ╚══════╝╚══════╝╚══════╝╚═╝╚══════╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝   ╚═╝   \n";
    std::cout << "\n";
    std::cout << "                    Multi-Agent Development Assistant\n";
    std::cout << "                         REST API Server v1.0\n";
    std::cout << "\n";
    
    try
    {
        server_instance = std::make_unique<ApiServer>(config);
        
        std::cout << "\n🚀 Server starting...\n\n";
        
        if (!server_instance->start())
        {
            std::cerr << "❌ Failed to start server\n";
            return 1;
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "\n❌ Server error: " << e.what() << "\n\n";
        return 1;
    }
    
    return 0;
}
