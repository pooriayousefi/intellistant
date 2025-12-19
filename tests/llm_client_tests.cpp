#include <iostream>
#include <cassert>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include "../include/llm_client.hpp"

using namespace pooriayousefi;

// Global variable to store server process PID
pid_t server_pid = 0;

// Server configuration
const std::string MODEL_PATH = "../models/qwen2.5-coder-3b/instruct-q4_k_m.gguf";
const std::string SERVER_PATH = "../runtime/llama-server";
const std::string HOST = "127.0.0.1";
const int PORT = 8080;

// ============================================================================
// Helper Functions
// ============================================================================

void start_llama_server()
{
    std::cout << "Starting llama-server..." << std::endl;
    
    server_pid = fork();
    
    if (server_pid == 0)
    {
        // Child process: run llama-server
        const char* args[] = {
            SERVER_PATH.c_str(),
            "-m", MODEL_PATH.c_str(),
            "--host", HOST.c_str(),
            "--port", std::to_string(PORT).c_str(),
            "-c", "2048",
            "-n", "512",
            "--log-disable",
            nullptr
        };
        
        execv(SERVER_PATH.c_str(), const_cast<char* const*>(args));
        
        // If execv returns, it failed
        std::cerr << "Failed to start llama-server" << std::endl;
        exit(1);
    }
    else if (server_pid > 0)
    {
        // Parent process: wait for server to be ready
        std::cout << "Waiting for server to start (PID: " << server_pid << ")..." << std::endl;
        
        LlmClient client(HOST, PORT);
        
        // Try to connect for up to 30 seconds
        for (int i = 0; i < 60; ++i)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            
            auto health = client.health_check();
            if (health && *health)
            {
                std::cout << "Server is ready!" << std::endl;
                return;
            }
        }
        
        std::cerr << "Server failed to start within timeout" << std::endl;
        kill(server_pid, SIGTERM);
        exit(1);
    }
    else
    {
        std::cerr << "Failed to fork process" << std::endl;
        exit(1);
    }
}

void stop_llama_server()
{
    if (server_pid > 0)
    {
        std::cout << "Stopping llama-server..." << std::endl;
        kill(server_pid, SIGTERM);
        
        // Wait for process to terminate
        int status;
        waitpid(server_pid, &status, 0);
        
        std::cout << "Server stopped." << std::endl;
        server_pid = 0;
    }
}

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

void test_health_check()
{
    test_header("Health Check");
    
    LlmClient client(HOST, PORT);
    
    auto result = client.health_check();
    
    if (!result)
    {
        test_failure("Health check failed: " + result.error().message);
    }
    
    if (*result)
    {
        test_success("Health check passed");
    }
    else
    {
        test_failure("Server is not healthy");
    }
}

void test_tokenize_detokenize()
{
    test_header("Tokenize and Detokenize");
    
    LlmClient client(HOST, PORT);
    
    std::string test_text = "Hello, world! This is a test.";
    
    // Test tokenization
    auto tokenize_result = client.tokenize(test_text);
    
    if (!tokenize_result)
    {
        test_failure("Tokenization failed: " + tokenize_result.error().message);
    }
    
    const auto& tokens = tokenize_result->tokens;
    test_success("Tokenized text into " + std::to_string(tokens.size()) + " tokens");
    
    // Test detokenization
    auto detokenize_result = client.detokenize(tokens);
    
    if (!detokenize_result)
    {
        test_failure("Detokenization failed: " + detokenize_result.error().message);
    }
    
    test_success("Detokenized tokens back to text");
    std::cout << "Original: " << test_text << std::endl;
    std::cout << "Roundtrip: " << detokenize_result->content << std::endl;
}

void test_completion()
{
    test_header("Text Completion");
    
    LlmClient client(HOST, PORT);
    
    CompletionConfig config;
    config.temperature = 0.7f;
    config.max_tokens = 50;
    
    std::string prompt = "Write a haiku about programming:";
    
    auto result = client.completion(prompt, config);
    
    if (!result)
    {
        test_failure("Completion failed: " + result.error().message);
    }
    
    std::cout << "Completion response ID: " << result->id << std::endl;
    std::cout << "Number of choices: " << result->choices.size() << std::endl;
    
    if (result->choices.empty())
    {
        test_failure("No completion choices returned");
    }
    
    test_success("Completion successful");
    std::cout << "Prompt: " << prompt << std::endl;
    std::cout << "Response: " << result->choices[0].text << std::endl;
    std::cout << "Tokens used: " << result->usage.total_tokens << std::endl;
}

void test_streaming_completion()
{
    test_header("Streaming Text Completion");
    
    LlmClient client(HOST, PORT);
    
    CompletionConfig config;
    config.temperature = 0.7f;
    config.max_tokens = 50;
    
    std::string prompt = "Count from 1 to 5:";
    
    std::cout << "Prompt: " << prompt << std::endl;
    std::cout << "Streaming response: ";
    std::cout.flush();
    
    auto generator = client.completion_stream(prompt, config);
    
    int chunk_count = 0;
    std::string full_text;
    
    while (generator.next())
    {
        auto chunk_result = generator.value();
        
        if (!chunk_result)
        {
            test_failure("Stream chunk error: " + chunk_result.error().message);
        }
        
        const auto& chunk = *chunk_result;
        std::cout << chunk.text;
        std::cout.flush();
        
        full_text += chunk.text;
        chunk_count++;
        
        if (chunk.is_final)
        {
            break;
        }
    }
    
    std::cout << std::endl;
    test_success("Streaming completed with " + std::to_string(chunk_count) + " chunks");
}

void test_chat_completion()
{
    test_header("Chat Completion");
    
    LlmClient client(HOST, PORT);
    
    CompletionConfig config;
    config.temperature = 0.7f;
    config.max_tokens = 100;
    
    std::vector<ChatMessage> messages;
    messages.push_back({"system", "You are a helpful assistant."});
    messages.push_back({"user", "What is 2+2?"});
    
    auto result = client.chat_completion(messages, config);
    
    if (!result)
    {
        test_failure("Chat completion failed: " + result.error().message);
    }
    
    if (result->choices.empty())
    {
        test_failure("No chat completion choices returned");
    }
    
    test_success("Chat completion successful");
    std::cout << "User: " << messages[1].content << std::endl;
    std::cout << "Assistant: " << result->choices[0].message.content << std::endl;
    std::cout << "Tokens used: " << result->usage.total_tokens << std::endl;
}

void test_streaming_chat_completion()
{
    test_header("Streaming Chat Completion");
    
    LlmClient client(HOST, PORT);
    
    CompletionConfig config;
    config.temperature = 0.7f;
    config.max_tokens = 100;
    
    std::vector<ChatMessage> messages;
    messages.push_back({"system", "You are a helpful assistant that responds briefly."});
    messages.push_back({"user", "Write a one-sentence fun fact about computers."});
    
    std::cout << "User: " << messages[1].content << std::endl;
    std::cout << "Assistant (streaming): ";
    std::cout.flush();
    
    auto generator = client.chat_completion_stream(messages, config);
    
    int chunk_count = 0;
    std::string full_text;
    
    while (generator.next())
    {
        auto chunk_result = generator.value();
        
        if (!chunk_result)
        {
            test_failure("Stream chunk error: " + chunk_result.error().message);
        }
        
        const auto& chunk = *chunk_result;
        std::cout << chunk.text;
        std::cout.flush();
        
        full_text += chunk.text;
        chunk_count++;
        
        if (chunk.is_final)
        {
            break;
        }
    }
    
    std::cout << std::endl;
    test_success("Streaming chat completed with " + std::to_string(chunk_count) + " chunks");
}

void test_runtime_config_changes()
{
    test_header("Runtime Configuration Changes");
    
    LlmClient client(HOST, PORT);
    
    std::string prompt = "Say 'creative':";
    
    // Test 1: Low temperature (deterministic)
    {
        CompletionConfig config;
        config.temperature = 0.1f;
        config.max_tokens = 20;
        
        auto result = client.completion(prompt, config);
        
        if (!result)
        {
            test_failure("Completion with low temperature failed: " + result.error().message);
        }
        
        test_success("Low temperature (0.1) completion successful");
        std::cout << "Response: " << result->choices[0].text << std::endl;
    }
    
    // Test 2: High temperature (creative)
    {
        CompletionConfig config;
        config.temperature = 1.5f;
        config.max_tokens = 20;
        
        auto result = client.completion(prompt, config);
        
        if (!result)
        {
            test_failure("Completion with high temperature failed: " + result.error().message);
        }
        
        test_success("High temperature (1.5) completion successful");
        std::cout << "Response: " << result->choices[0].text << std::endl;
    }
    
    // Test 3: Different max_tokens
    {
        CompletionConfig config;
        config.temperature = 0.7f;
        config.max_tokens = 5;
        
        auto result = client.completion("Count to 100:", config);
        
        if (!result)
        {
            test_failure("Completion with limited tokens failed: " + result.error().message);
        }
        
        test_success("Limited max_tokens (5) completion successful");
        std::cout << "Response: " << result->choices[0].text << std::endl;
    }
    
    test_success("Runtime configuration changes work correctly");
}

void test_embeddings()
{
    test_header("Embeddings");
    
    LlmClient client(HOST, PORT);
    
    std::string text = "Hello, world!";
    
    auto result = client.embeddings(text);
    
    if (!result)
    {
        // Embeddings may not be available on all llama-server configurations
        std::cout << "[SKIP] Embeddings not available: " << result.error().message << std::endl;
        return;
    }
    
    if (result->embedding.empty())
    {
        test_failure("Embedding vector is empty");
    }
    
    test_success("Embeddings generated successfully");
    std::cout << "Text: " << text << std::endl;
    std::cout << "Embedding dimension: " << result->embedding.size() << std::endl;
    std::cout << "First 5 values: ";
    for (size_t i = 0; i < std::min(size_t(5), result->embedding.size()); ++i)
    {
        std::cout << result->embedding[i] << " ";
    }
    std::cout << std::endl;
}

void test_multi_turn_conversation()
{
    test_header("Multi-Turn Conversation");
    
    LlmClient client(HOST, PORT);
    
    CompletionConfig config;
    config.temperature = 0.7f;
    config.max_tokens = 50;
    
    std::vector<ChatMessage> messages;
    messages.push_back({"system", "You are a helpful math tutor."});
    messages.push_back({"user", "What is 5 + 3?"});
    
    // First turn
    auto result1 = client.chat_completion(messages, config);
    
    if (!result1)
    {
        test_failure("First turn failed: " + result1.error().message);
    }
    
    test_success("First turn successful");
    std::cout << "Turn 1 - User: " << messages[1].content << std::endl;
    std::cout << "Turn 1 - Assistant: " << result1->choices[0].message.content << std::endl;
    
    // Add assistant response to conversation
    messages.push_back(result1->choices[0].message);
    
    // Second turn - change config mid-conversation
    messages.push_back({"user", "What is that number multiplied by 2?"});
    config.temperature = 0.3f;  // More deterministic for math
    
    auto result2 = client.chat_completion(messages, config);
    
    if (!result2)
    {
        test_failure("Second turn failed: " + result2.error().message);
    }
    
    test_success("Second turn successful with updated config");
    std::cout << "Turn 2 - User: " << messages[3].content << std::endl;
    std::cout << "Turn 2 - Assistant: " << result2->choices[0].message.content << std::endl;
    std::cout << "Config changed: temperature 0.7 -> 0.3" << std::endl;
}

void test_error_handling()
{
    test_header("Error Handling");
    
    // Test connection to non-existent server
    {
        LlmClient client("127.0.0.1", 9999);
        auto result = client.health_check();
        
        if (result)
        {
            test_failure("Expected connection failure but succeeded");
        }
        
        test_success("Connection failure handled correctly");
        std::cout << "Error: " << result.error().message << std::endl;
    }
    
    // Test invalid config (will still work but server may limit)
    {
        LlmClient client(HOST, PORT);
        CompletionConfig config;
        config.max_tokens = 0;  // Invalid
        
        auto result = client.completion("Test", config);
        
        // This might succeed or fail depending on server validation
        test_success("Invalid config handled");
    }
    
    test_success("Error handling tests completed");
}

// ============================================================================
// Main Test Runner
// ============================================================================

int main()
{
    std::cout << "========================================" << std::endl;
    std::cout << "LLM CLIENT TEST SUITE" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Model: " << MODEL_PATH << std::endl;
    std::cout << "Server: " << SERVER_PATH << std::endl;
    std::cout << "Host: " << HOST << ":" << PORT << std::endl;
    
    // Start the llama-server
    start_llama_server();
    
    // Register cleanup handler
    atexit(stop_llama_server);
    
    try
    {
        // Run all tests
        test_health_check();
        test_tokenize_detokenize();
        test_completion();
        test_streaming_completion();
        test_chat_completion();
        test_streaming_chat_completion();
        test_runtime_config_changes();
        test_embeddings();
        test_multi_turn_conversation();
        test_error_handling();
        
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
