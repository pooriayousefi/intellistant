#ifndef POORIAYOUSEFI_LLM_CLIENT_HPP
#define POORIAYOUSEFI_LLM_CLIENT_HPP

#include <string>
#include <vector>
#include <map>
#include <optional>
#include <expected>
#include <coroutine>
#include <memory>
#include <chrono>
#include <functional>
#include <sstream>
#include <exception>
#include "httplib.h"
#include "../jsonrpc2/include/json.hpp"

namespace pooriayousefi
{

    // ============================================================================
    // Chat Roles
    // ============================================================================

    struct ChatRole
    {
        static constexpr const char* System = "system";
        static constexpr const char* User = "user";
        static constexpr const char* Assistant = "assistant";
        static constexpr const char* Tool = "tool";
    };

    // ============================================================================
    // Error Handling
    // ============================================================================

    enum class LlmErrorCode
    {
        connection_failed,
        request_timeout,
        invalid_response,
        server_error,
        invalid_config,
        model_not_loaded,
        tokenization_failed,
        unknown_error
    };

    struct LlmError
    {
        LlmErrorCode code;
        std::string message;
        int http_status{0};

        LlmError(LlmErrorCode c, std::string msg, int status = 0)
            : code(c), message(std::move(msg)), http_status(status)
        {
        }
    };

    template<typename T>
    using Result = std::expected<T, LlmError>;

    // ============================================================================
    // Configuration Structures
    // ============================================================================

    struct CompletionConfig
    {
        std::optional<float> temperature;
        std::optional<float> top_p;
        std::optional<int> top_k;
        std::optional<int> max_tokens;
        std::optional<int> min_tokens;
        std::optional<float> repeat_penalty;
        std::optional<int> repeat_last_n;
        std::optional<float> presence_penalty;
        std::optional<float> frequency_penalty;
        std::optional<int> seed;
        std::vector<std::string> stop;
        bool stream{false};

        nlohmann::json to_json() const
        {
            nlohmann::json j;
            if (temperature) j["temperature"] = *temperature;
            if (top_p) j["top_p"] = *top_p;
            if (top_k) j["top_k"] = *top_k;
            if (max_tokens) j["max_tokens"] = *max_tokens;
            if (min_tokens) j["min_tokens"] = *min_tokens;
            if (repeat_penalty) j["repeat_penalty"] = *repeat_penalty;
            if (repeat_last_n) j["repeat_last_n"] = *repeat_last_n;
            if (presence_penalty) j["presence_penalty"] = *presence_penalty;
            if (frequency_penalty) j["frequency_penalty"] = *frequency_penalty;
            if (seed) j["seed"] = *seed;
            if (!stop.empty()) j["stop"] = stop;
            j["stream"] = stream;
            return j;
        }
    };

    // ============================================================================
    // Response Structures
    // ============================================================================

    // Tool calling structures
    struct ToolCallFunction
    {
        std::string name;
        nlohmann::json arguments;

        nlohmann::json to_json() const
        {
            return nlohmann::json{
                {"name", name},
                {"arguments", arguments}
            };
        }

        static ToolCallFunction from_json(const nlohmann::json& j)
        {
            ToolCallFunction func;
            if (j.contains("name")) func.name = j["name"];
            if (j.contains("arguments"))
            {
                if (j["arguments"].is_string())
                {
                    func.arguments = nlohmann::json::parse(j["arguments"].get<std::string>());
                }
                else
                {
                    func.arguments = j["arguments"];
                }
            }
            return func;
        }
    };

    struct ToolCall
    {
        std::string id;
        std::string type;
        ToolCallFunction function;

        nlohmann::json to_json() const
        {
            return nlohmann::json{
                {"id", id},
                {"type", type},
                {"function", function.to_json()}
            };
        }

        static ToolCall from_json(const nlohmann::json& j)
        {
            ToolCall call;
            if (j.contains("id")) call.id = j["id"];
            if (j.contains("type")) call.type = j["type"];
            if (j.contains("function")) call.function = ToolCallFunction::from_json(j["function"]);
            return call;
        }
    };

    struct ChatMessage
    {
        std::string role;
        std::string content;
        std::vector<ToolCall> tool_calls;
        std::string tool_call_id;  // For tool role messages

        nlohmann::json to_json() const
        {
            nlohmann::json j;
            j["role"] = role;
            if (!content.empty()) j["content"] = content;
            if (!tool_calls.empty())
            {
                nlohmann::json calls = nlohmann::json::array();
                for (const auto& call : tool_calls)
                {
                    calls.push_back(call.to_json());
                }
                j["tool_calls"] = calls;
            }
            if (!tool_call_id.empty()) j["tool_call_id"] = tool_call_id;
            return j;
        }

        static ChatMessage from_json(const nlohmann::json& j)
        {
            ChatMessage msg;
            msg.role = j.at("role").get<std::string>();
            if (j.contains("content") && !j["content"].is_null())
            {
                msg.content = j["content"].get<std::string>();
            }
            if (j.contains("tool_calls") && j["tool_calls"].is_array())
            {
                for (const auto& call_json : j["tool_calls"])
                {
                    msg.tool_calls.push_back(ToolCall::from_json(call_json));
                }
            }
            if (j.contains("tool_call_id"))
            {
                msg.tool_call_id = j["tool_call_id"];
            }
            return msg;
        }
    };

    struct CompletionUsage
    {
        int prompt_tokens{0};
        int completion_tokens{0};
        int total_tokens{0};

        static CompletionUsage from_json(const nlohmann::json& j)
        {
            CompletionUsage usage;
            if (j.contains("prompt_tokens")) usage.prompt_tokens = j["prompt_tokens"];
            if (j.contains("completion_tokens")) usage.completion_tokens = j["completion_tokens"];
            if (j.contains("total_tokens")) usage.total_tokens = j["total_tokens"];
            return usage;
        }
    };

    struct CompletionChoice
    {
        std::string text;
        int index{0};
        std::string finish_reason;

        static CompletionChoice from_json(const nlohmann::json& j)
        {
            CompletionChoice choice;
            if (j.contains("text")) choice.text = j["text"];
            if (j.contains("index")) choice.index = j["index"];
            if (j.contains("finish_reason")) choice.finish_reason = j["finish_reason"];
            return choice;
        }
    };

    struct CompletionResponse
    {
        std::string id;
        std::string object;
        int64_t created{0};
        std::string model;
        std::vector<CompletionChoice> choices;
        CompletionUsage usage;

        static Result<CompletionResponse> from_json(const nlohmann::json& j)
        {
            try
            {
                CompletionResponse response;
                if (j.contains("id")) response.id = j["id"];
                if (j.contains("object")) response.object = j["object"];
                if (j.contains("created")) response.created = j["created"];
                if (j.contains("model")) response.model = j["model"];
                
                if (j.contains("choices") && j["choices"].is_array())
                {
                    for (const auto& choice_json : j["choices"])
                    {
                        response.choices.push_back(CompletionChoice::from_json(choice_json));
                    }
                }
                
                // Handle "content" field if "choices" is empty (llama-server may use this)
                if (response.choices.empty() && j.contains("content"))
                {
                    CompletionChoice choice;
                    choice.text = j["content"];
                    choice.index = 0;
                    choice.finish_reason = j.value("stop_reason", "");
                    response.choices.push_back(choice);
                }
                
                if (j.contains("usage"))
                {
                    response.usage = CompletionUsage::from_json(j["usage"]);
                }
                
                return response;
            }
            catch (const std::exception& e)
            {
                return std::unexpected(LlmError(
                    LlmErrorCode::invalid_response,
                    std::string("Failed to parse completion response: ") + e.what()
                ));
            }
        }
    };

    struct ChatCompletionChoice
    {
        ChatMessage message;
        int index{0};
        std::string finish_reason;

        static ChatCompletionChoice from_json(const nlohmann::json& j)
        {
            ChatCompletionChoice choice;
            if (j.contains("message")) choice.message = ChatMessage::from_json(j["message"]);
            if (j.contains("index")) choice.index = j["index"];
            if (j.contains("finish_reason") && !j["finish_reason"].is_null())
            {
                choice.finish_reason = j["finish_reason"];
            }
            return choice;
        }
    };

    struct ChatCompletionResponse
    {
        std::string id;
        std::string object;
        int64_t created{0};
        std::string model;
        std::vector<ChatCompletionChoice> choices;
        CompletionUsage usage;

        static Result<ChatCompletionResponse> from_json(const nlohmann::json& j)
        {
            try
            {
                ChatCompletionResponse response;
                if (j.contains("id")) response.id = j["id"];
                if (j.contains("object")) response.object = j["object"];
                if (j.contains("created")) response.created = j["created"];
                if (j.contains("model")) response.model = j["model"];
                
                if (j.contains("choices") && j["choices"].is_array())
                {
                    for (const auto& choice_json : j["choices"])
                    {
                        response.choices.push_back(ChatCompletionChoice::from_json(choice_json));
                    }
                }
                
                if (j.contains("usage"))
                {
                    response.usage = CompletionUsage::from_json(j["usage"]);
                }
                
                return response;
            }
            catch (const std::exception& e)
            {
                return std::unexpected(LlmError(
                    LlmErrorCode::invalid_response,
                    std::string("Failed to parse chat completion response: ") + e.what()
                ));
            }
        }
    };

    struct StreamChunk
    {
        std::string text;
        std::string finish_reason;
        bool is_final{false};

        static Result<StreamChunk> from_json(const nlohmann::json& j)
        {
            try
            {
                StreamChunk chunk;
                
                if (j.contains("choices") && j["choices"].is_array() && !j["choices"].empty())
                {
                    const auto& choice = j["choices"][0];
                    
                    if (choice.contains("text") && !choice["text"].is_null())
                    {
                        chunk.text = choice["text"];
                    }
                    else if (choice.contains("delta") && choice["delta"].is_object())
                    {
                        if (choice["delta"].contains("content") && !choice["delta"]["content"].is_null())
                        {
                            chunk.text = choice["delta"]["content"];
                        }
                    }
                    
                    if (choice.contains("finish_reason") && !choice["finish_reason"].is_null())
                    {
                        chunk.finish_reason = choice["finish_reason"];
                        chunk.is_final = true;
                    }
                }
                
                return chunk;
            }
            catch (const std::exception& e)
            {
                return std::unexpected(LlmError(
                    LlmErrorCode::invalid_response,
                    std::string("Failed to parse stream chunk: ") + e.what()
                ));
            }
        }
    };

    struct TokenizeResponse
    {
        std::vector<int> tokens;

        static Result<TokenizeResponse> from_json(const nlohmann::json& j)
        {
            try
            {
                TokenizeResponse response;
                if (j.contains("tokens") && j["tokens"].is_array())
                {
                    response.tokens = j["tokens"].get<std::vector<int>>();
                }
                return response;
            }
            catch (const std::exception& e)
            {
                return std::unexpected(LlmError(
                    LlmErrorCode::tokenization_failed,
                    std::string("Failed to parse tokenize response: ") + e.what()
                ));
            }
        }
    };

    struct DetokenizeResponse
    {
        std::string content;

        static Result<DetokenizeResponse> from_json(const nlohmann::json& j)
        {
            try
            {
                DetokenizeResponse response;
                if (j.contains("content"))
                {
                    response.content = j["content"];
                }
                return response;
            }
            catch (const std::exception& e)
            {
                return std::unexpected(LlmError(
                    LlmErrorCode::tokenization_failed,
                    std::string("Failed to parse detokenize response: ") + e.what()
                ));
            }
        }
    };

    struct EmbeddingResponse
    {
        std::vector<float> embedding;

        static Result<EmbeddingResponse> from_json(const nlohmann::json& j)
        {
            try
            {
                EmbeddingResponse response;
                if (j.contains("embedding") && j["embedding"].is_array())
                {
                    response.embedding = j["embedding"].get<std::vector<float>>();
                }
                else if (j.contains("data") && j["data"].is_array() && !j["data"].empty())
                {
                    const auto& first = j["data"][0];
                    if (first.contains("embedding"))
                    {
                        response.embedding = first["embedding"].get<std::vector<float>>();
                    }
                }
                return response;
            }
            catch (const std::exception& e)
            {
                return std::unexpected(LlmError(
                    LlmErrorCode::invalid_response,
                    std::string("Failed to parse embedding response: ") + e.what()
                ));
            }
        }
    };

    // ============================================================================
    // Streaming Generator
    // ============================================================================

    template<typename T>
    struct Generator
    {
        struct promise_type
        {
            std::optional<T> current_value;
            std::exception_ptr exception;

            Generator get_return_object()
            {
                return Generator{std::coroutine_handle<promise_type>::from_promise(*this)};
            }

            std::suspend_always initial_suspend() noexcept { return {}; }
            std::suspend_always final_suspend() noexcept { return {}; }

            std::suspend_always yield_value(T value) noexcept
            {
                current_value = std::move(value);
                return {};
            }

            void return_void() noexcept {}

            void unhandled_exception() noexcept
            {
                exception = std::current_exception();
            }
        };

        using handle_type = std::coroutine_handle<promise_type>;

        class Iterator
        {
        public:
            using iterator_category = std::input_iterator_tag;
            using value_type = T;
            using difference_type = std::ptrdiff_t;
            using pointer = T*;
            using reference = T&;

            Iterator() = default;

            explicit Iterator(handle_type h) : handle_(h) {}

            Iterator& operator++()
            {
                if (handle_ && !handle_.done())
                {
                    handle_.resume();
                    if (handle_.done() || !handle_.promise().current_value)
                    {
                        handle_ = nullptr;
                    }
                }
                return *this;
            }

            void operator++(int)
            {
                ++(*this);
            }

            T& operator*() const
            {
                return *handle_.promise().current_value;
            }

            T* operator->() const
            {
                return &(*handle_.promise().current_value);
            }

            bool operator==(const Iterator& other) const
            {
                return handle_ == other.handle_;
            }

            bool operator!=(const Iterator& other) const
            {
                return !(*this == other);
            }

        private:
            handle_type handle_ = nullptr;
        };

        Generator() = default;

        explicit Generator(handle_type h) : coro(h) {}

        Generator(Generator&& other) noexcept
            : coro(std::exchange(other.coro, nullptr))
        {
        }

        Generator& operator=(Generator&& other) noexcept
        {
            if (this != &other)
            {
                if (coro)
                {
                    coro.destroy();
                }
                coro = std::exchange(other.coro, nullptr);
            }
            return *this;
        }

        ~Generator()
        {
            if (coro)
            {
                coro.destroy();
            }
        }

        Generator(const Generator&) = delete;
        Generator& operator=(const Generator&) = delete;

        // Iterator support for range-based for loops
        Iterator begin()
        {
            if (coro)
            {
                coro.resume();
                if (coro.promise().exception)
                {
                    std::rethrow_exception(coro.promise().exception);
                }
                if (coro.done() || !coro.promise().current_value)
                {
                    return end();
                }
                return Iterator{coro};
            }
            return end();
        }

        Iterator end()
        {
            return Iterator{};
        }

        // Legacy API for backward compatibility
        bool next()
        {
            if (!coro || coro.done())
            {
                return false;
            }

            coro.resume();

            if (coro.promise().exception)
            {
                std::rethrow_exception(coro.promise().exception);
            }

            return !coro.done();
        }

        T value() const
        {
            if (coro.promise().current_value)
            {
                return *coro.promise().current_value;
            }
            throw std::runtime_error("No value available");
        }

    private:
        handle_type coro = nullptr;
    };

    // ============================================================================
    // LLM Client
    // ============================================================================

    class LlmClient
    {
    public:
        LlmClient(
            const std::string& host = "127.0.0.1",
            int port = 8080,
            std::chrono::seconds timeout = std::chrono::seconds(300)
        )
            : host_(host)
            , port_(port)
            , timeout_(timeout)
        {
            client_ = std::make_unique<httplib::Client>(host_, port_);
            client_->set_read_timeout(timeout_);
            client_->set_write_timeout(timeout_);
        }

        // Health check
        Result<bool> health_check()
        {
            auto res = client_->Get("/health");
            
            if (!res)
            {
                return std::unexpected(LlmError(
                    LlmErrorCode::connection_failed,
                    "Failed to connect to server"
                ));
            }

            if (res->status != 200)
            {
                return std::unexpected(LlmError(
                    LlmErrorCode::server_error,
                    "Server health check failed",
                    res->status
                ));
            }

            try
            {
                auto j = nlohmann::json::parse(res->body);
                if (j.contains("status") && j["status"] == "ok")
                {
                    return true;
                }
                return false;
            }
            catch (const std::exception& e)
            {
                return std::unexpected(LlmError(
                    LlmErrorCode::invalid_response,
                    std::string("Invalid health check response: ") + e.what()
                ));
            }
        }

        // Text completion (non-streaming)
        Result<CompletionResponse> completion(
            const std::string& prompt,
            CompletionConfig config = {}
        )
        {
            config.stream = false;
            
            nlohmann::json request_body = config.to_json();
            request_body["prompt"] = prompt;

            auto res = client_->Post(
                "/completion",
                request_body.dump(),
                "application/json"
            );

            return handle_completion_response(res);
        }

        // Text completion (streaming)
        Generator<Result<StreamChunk>> completion_stream(
            const std::string& prompt,
            CompletionConfig config = {}
        )
        {
            config.stream = true;
            
            nlohmann::json request_body = config.to_json();
            request_body["prompt"] = prompt;

            std::string accumulated_data;

            httplib::Headers headers;
            auto res = client_->Post(
                "/completion",
                headers,
                request_body.dump(),
                "application/json",
                [&](const char* data, size_t data_length) -> bool
                {
                    accumulated_data.append(data, data_length);
                    return true;
                }
            );

            if (!res)
            {
                co_yield std::unexpected(LlmError(
                    LlmErrorCode::connection_failed,
                    "Failed to connect to server"
                ));
                co_return;
            }

            if (res->status != 200)
            {
                co_yield std::unexpected(LlmError(
                    LlmErrorCode::server_error,
                    "Server returned error status",
                    res->status
                ));
                co_return;
            }

            // Parse SSE stream
            std::istringstream stream(accumulated_data);
            std::string line;
            std::string data_buffer;

            while (std::getline(stream, line))
            {
                if (line.empty() || line == "\r")
                {
                    if (!data_buffer.empty())
                    {
                        try
                        {
                            auto j = nlohmann::json::parse(data_buffer);
                            auto chunk_result = StreamChunk::from_json(j);
                            
                            if (chunk_result)
                            {
                                co_yield chunk_result.value();
                            }
                            else
                            {
                                co_yield std::unexpected(chunk_result.error());
                            }
                        }
                        catch (const std::exception& e)
                        {
                            // Skip invalid chunks
                        }
                        data_buffer.clear();
                    }
                }
                else if (line.rfind("data: ", 0) == 0)
                {
                    data_buffer = line.substr(6);
                }
            }

            co_return;
        }

        // Chat completion (non-streaming)
        Result<ChatCompletionResponse> chat_completion(
            const std::vector<ChatMessage>& messages,
            CompletionConfig config = {}
        )
        {
            config.stream = false;
            
            nlohmann::json request_body = config.to_json();
            nlohmann::json messages_json = nlohmann::json::array();
            for (const auto& msg : messages)
            {
                messages_json.push_back(msg.to_json());
            }
            request_body["messages"] = messages_json;

            auto res = client_->Post(
                "/v1/chat/completions",
                request_body.dump(),
                "application/json"
            );

            return handle_chat_completion_response(res);
        }

        // Chat completion (streaming)
        Generator<Result<StreamChunk>> chat_completion_stream(
            const std::vector<ChatMessage>& messages,
            CompletionConfig config = {}
        )
        {
            config.stream = true;
            
            nlohmann::json request_body = config.to_json();
            nlohmann::json messages_json = nlohmann::json::array();
            for (const auto& msg : messages)
            {
                messages_json.push_back(msg.to_json());
            }
            request_body["messages"] = messages_json;

            std::string accumulated_data;

            httplib::Headers headers;
            auto res = client_->Post(
                "/v1/chat/completions",
                headers,
                request_body.dump(),
                "application/json",
                [&](const char* data, size_t data_length) -> bool
                {
                    accumulated_data.append(data, data_length);
                    return true;
                }
            );

            if (!res)
            {
                co_yield std::unexpected(LlmError(
                    LlmErrorCode::connection_failed,
                    "Failed to connect to server"
                ));
                co_return;
            }

            if (res->status != 200)
            {
                co_yield std::unexpected(LlmError(
                    LlmErrorCode::server_error,
                    "Server returned error status",
                    res->status
                ));
                co_return;
            }

            // Parse SSE stream
            std::istringstream stream(accumulated_data);
            std::string line;
            std::string data_buffer;

            while (std::getline(stream, line))
            {
                if (line.empty() || line == "\r")
                {
                    if (!data_buffer.empty())
                    {
                        if (data_buffer == "[DONE]")
                        {
                            co_return;
                        }

                        try
                        {
                            auto j = nlohmann::json::parse(data_buffer);
                            auto chunk_result = StreamChunk::from_json(j);
                            
                            if (chunk_result)
                            {
                                co_yield chunk_result.value();
                            }
                            else
                            {
                                co_yield std::unexpected(chunk_result.error());
                            }
                        }
                        catch (const std::exception& e)
                        {
                            // Skip invalid chunks
                        }
                        data_buffer.clear();
                    }
                }
                else if (line.rfind("data: ", 0) == 0)
                {
                    data_buffer = line.substr(6);
                }
            }

            co_return;
        }

        // Chat completion with tool calling support
        Result<ChatMessage> chat_completion_with_tools(
            const std::vector<ChatMessage>& messages,
            const std::vector<nlohmann::json>& tools,
            CompletionConfig config = {}
        )
        {
            config.stream = false;
            
            nlohmann::json request_body = config.to_json();
            nlohmann::json messages_json = nlohmann::json::array();
            for (const auto& msg : messages)
            {
                messages_json.push_back(msg.to_json());
            }
            request_body["messages"] = messages_json;
            
            // Add tools if provided
            if (!tools.empty())
            {
                request_body["tools"] = tools;
                request_body["tool_choice"] = "auto";
            }

            auto res = client_->Post(
                "/v1/chat/completions",
                request_body.dump(),
                "application/json"
            );

            if (!res)
            {
                return std::unexpected(LlmError(
                    LlmErrorCode::connection_failed,
                    "Failed to connect to server"
                ));
            }

            if (res->status != 200)
            {
                return std::unexpected(LlmError(
                    LlmErrorCode::server_error,
                    "Chat completion failed",
                    res->status
                ));
            }

            try
            {
                auto j = nlohmann::json::parse(res->body);
                auto response_result = ChatCompletionResponse::from_json(j);
                
                if (!response_result)
                {
                    return std::unexpected(response_result.error());
                }
                
                auto& response = response_result.value();
                
                if (response.choices.empty())
                {
                    return std::unexpected(LlmError(
                        LlmErrorCode::invalid_response,
                        "No choices in response"
                    ));
                }
                
                return response.choices[0].message;
            }
            catch (const std::exception& e)
            {
                return std::unexpected(LlmError(
                    LlmErrorCode::invalid_response,
                    std::string("Failed to parse response: ") + e.what()
                ));
            }
        }

        // Tokenize text
        Result<TokenizeResponse> tokenize(const std::string& content)
        {
            nlohmann::json request_body;
            request_body["content"] = content;

            auto res = client_->Post(
                "/tokenize",
                request_body.dump(),
                "application/json"
            );

            if (!res)
            {
                return std::unexpected(LlmError(
                    LlmErrorCode::connection_failed,
                    "Failed to connect to server"
                ));
            }

            if (res->status != 200)
            {
                return std::unexpected(LlmError(
                    LlmErrorCode::server_error,
                    "Tokenization failed",
                    res->status
                ));
            }

            try
            {
                auto j = nlohmann::json::parse(res->body);
                return TokenizeResponse::from_json(j);
            }
            catch (const std::exception& e)
            {
                return std::unexpected(LlmError(
                    LlmErrorCode::invalid_response,
                    std::string("Failed to parse response: ") + e.what()
                ));
            }
        }

        // Detokenize tokens
        Result<DetokenizeResponse> detokenize(const std::vector<int>& tokens)
        {
            nlohmann::json request_body;
            request_body["tokens"] = tokens;

            auto res = client_->Post(
                "/detokenize",
                request_body.dump(),
                "application/json"
            );

            if (!res)
            {
                return std::unexpected(LlmError(
                    LlmErrorCode::connection_failed,
                    "Failed to connect to server"
                ));
            }

            if (res->status != 200)
            {
                return std::unexpected(LlmError(
                    LlmErrorCode::server_error,
                    "Detokenization failed",
                    res->status
                ));
            }

            try
            {
                auto j = nlohmann::json::parse(res->body);
                return DetokenizeResponse::from_json(j);
            }
            catch (const std::exception& e)
            {
                return std::unexpected(LlmError(
                    LlmErrorCode::invalid_response,
                    std::string("Failed to parse response: ") + e.what()
                ));
            }
        }

        // Get embeddings
        Result<EmbeddingResponse> embeddings(const std::string& content)
        {
            nlohmann::json request_body;
            request_body["content"] = content;

            auto res = client_->Post(
                "/embedding",
                request_body.dump(),
                "application/json"
            );

            if (!res)
            {
                return std::unexpected(LlmError(
                    LlmErrorCode::connection_failed,
                    "Failed to connect to server"
                ));
            }

            if (res->status != 200)
            {
                return std::unexpected(LlmError(
                    LlmErrorCode::server_error,
                    "Embedding generation failed",
                    res->status
                ));
            }

            try
            {
                auto j = nlohmann::json::parse(res->body);
                return EmbeddingResponse::from_json(j);
            }
            catch (const std::exception& e)
            {
                return std::unexpected(LlmError(
                    LlmErrorCode::invalid_response,
                    std::string("Failed to parse response: ") + e.what()
                ));
            }
        }

    private:
        Result<CompletionResponse> handle_completion_response(
            const httplib::Result& res
        )
        {
            if (!res)
            {
                return std::unexpected(LlmError(
                    LlmErrorCode::connection_failed,
                    "Failed to connect to server"
                ));
            }

            if (res->status != 200)
            {
                return std::unexpected(LlmError(
                    LlmErrorCode::server_error,
                    "Server returned error status",
                    res->status
                ));
            }

            try
            {
                auto j = nlohmann::json::parse(res->body);
                return CompletionResponse::from_json(j);
            }
            catch (const std::exception& e)
            {
                return std::unexpected(LlmError(
                    LlmErrorCode::invalid_response,
                    std::string("Failed to parse response: ") + e.what()
                ));
            }
        }

        Result<ChatCompletionResponse> handle_chat_completion_response(
            const httplib::Result& res
        )
        {
            if (!res)
            {
                return std::unexpected(LlmError(
                    LlmErrorCode::connection_failed,
                    "Failed to connect to server"
                ));
            }

            if (res->status != 200)
            {
                return std::unexpected(LlmError(
                    LlmErrorCode::server_error,
                    "Server returned error status",
                    res->status
                ));
            }

            try
            {
                auto j = nlohmann::json::parse(res->body);
                return ChatCompletionResponse::from_json(j);
            }
            catch (const std::exception& e)
            {
                return std::unexpected(LlmError(
                    LlmErrorCode::invalid_response,
                    std::string("Failed to parse response: ") + e.what()
                ));
            }
        }

        std::string host_;
        int port_;
        std::chrono::seconds timeout_;
        std::unique_ptr<httplib::Client> client_;
    };

}  // namespace pooriayousefi

#endif  // POORIAYOUSEFI_LLM_CLIENT_HPP
