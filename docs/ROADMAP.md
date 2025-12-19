# Intellistant Framework - Development Roadmap

## Project Vision

**Intellistant** is a production-ready, multi-agent AI framework designed to serve as an intelligent assistant for software development company employees. The framework automates routine tasks, provides expert assistance, and enhances developer productivity through natural language interaction and tool execution.

## Core Design Principles

1. **Linux-First**: Optimized for Linux environments where development infrastructure runs
2. **Header-Only**: Minimal dependencies, easy deployment across teams
3. **Performance**: C++23 with coroutines, zero-copy where possible
4. **Real LLM Integration**: Direct llama.cpp integration for on-premise deployment
5. **Extensible Tool System**: Easy to add new tools for company-specific workflows
6. **Multi-Agent Architecture**: Specialized agents for different domains

## Target Users

- **Software Developers**: Code assistance, debugging, documentation
- **DevOps Engineers**: Infrastructure management, deployment automation
- **QA Engineers**: Test generation, bug analysis
- **Project Managers**: Status reports, task tracking
- **Technical Writers**: Documentation generation

---

## Architecture Overview

```
┌─────────────────────────────────────────────────────────────┐
│                         Coordinator                          │
│  • Routes requests to appropriate agents                     │
│  • Manages multi-agent collaboration                         │
│  • Handles context sharing between agents                    │
└─────────────────────────────────────────────────────────────┘
                            │
        ┌───────────────────┼───────────────────┐
        │                   │                   │
   ┌────▼────┐         ┌────▼────┐        ┌────▼────┐
   │ Agent 1 │         │ Agent 2 │        │ Agent N │
   │  Chat   │         │  Chat   │        │  Chat   │
   │  Tools  │         │  Tools  │        │  Tools  │
   └────┬────┘         └────┬────┘        └────┬────┘
        │                   │                   │
        └───────────────────┼───────────────────┘
                            │
                    ┌───────▼───────┐
                    │  LLM Client   │
                    │  (Phase 1 ✓)  │
                    └───────┬───────┘
                            │
                    ┌───────▼────────┐
                    │ llama.cpp      │
                    │ Inference      │
                    └────────────────┘
```

---

## Phase Breakdown

### ✅ Phase 1: LLM Client (COMPLETED)
**Status**: Complete and tested

**Deliverables**:
- `include/llm_client.hpp`
- `tests/llm_client_tests.cpp`
- CMake build system

**Features**:
- HTTP client for llama.cpp server
- Completion & chat APIs (streaming + blocking)
- Tokenization/detokenization
- Error handling with `std::expected`
- Runtime configuration (mutable parameters)
- C++20 coroutine-based streaming

---

### ✅ Phase 2: MCP-Based Tool System (COMPLETED)
**Status**: Complete and tested

**Deliverables**:
- `include/mcp_types.hpp` - MCP protocol types and schemas ✓
- `include/mcp_server.hpp` - MCP protocol server implementation ✓
- `include/mcp_tools.hpp` - Tool definition and execution framework (MCP-compliant) ✓
- `tests/mcp_tools_tests.cpp` - Comprehensive tool system tests ✓

**Implemented Tools** (12 total):

**File System Tools** (7):
- `read_file(path)` - Read file contents ✓
- `write_file(path, content)` - Write to file ✓
- `list_directory(path)` - List directory contents ✓
- `search_files(pattern, path)` - Search files by regex ✓
- `file_info(path)` - Get file metadata ✓
- `create_directory(path, parents)` - Create directories ✓
- `delete_path(path, recursive)` - Delete files/directories ✓

**Git Tools** (4):
- `git_status(repo_path)` - Get repository status ✓
- `git_log(repo_path, limit)` - Show commit history ✓
- `git_diff(repo_path, file)` - Show file changes ✓
- `git_branch_list(repo_path)` - List branches ✓

**System Tools** (1):
- `execute_command(cmd, working_dir)` - Run shell commands ✓

**Key Features**:
- Full JSON-RPC 2.0 protocol compliance
- JSON Schema parameter validation
- MCP protocol version 2024-11-05
- Function calling format for LLM integration
- Comprehensive error handling
- All tests passing ✓

---

### 🔄 Phase 3: Agent System
**Goal**: Implement specialized AI agents with chat and tool capabilities

#### Tool Categories for Software Developers

**1. File System Tools**
- `read_file(path)` - Read file contents
- `write_file(path, content)` - Write to file
- `list_directory(path)` - List directory contents
- `search_files(pattern, path)` - Search files by pattern
- `file_info(path)` - Get file metadata

**2. Git Tools**
- `git_status(repo_path)` - Get repository status
- `git_log(repo_path, limit)` - Show commit history
- `git_diff(repo_path, file)` - Show file changes
- `git_branch_list(repo_path)` - List branches
- `git_commit(repo_path, message)` - Commit changes

**3. Code Analysis Tools**
- `find_function(name, path)` - Locate function definition
- `find_usages(symbol, path)` - Find symbol references
- `analyze_dependencies(file)` - List file dependencies
- `check_syntax(file)` - Syntax validation
- `count_lines(path)` - LOC statistics

**4. Build Tools**
- `run_build(target)` - Execute build command
- `run_tests(pattern)` - Run test suite
- `check_errors()` - Parse compiler/linter errors
- `install_dependencies(package)` - Install packages

**5. Documentation Tools**
- `generate_docs(file)` - Generate documentation
- `explain_code(snippet)` - Explain code block
- `find_examples(api_name)` - Find usage examples

**6. System Tools**
- `execute_command(cmd)` - Run shell command
- `check_process(name)` - Check if process is running
- `system_info()` - Get system information
- `disk_usage(path)` - Check disk space

**7. Communication Tools**
- `send_notification(message)` - Desktop notification
- `create_issue(title, description)` - Create bug tracker issue
- `search_documentation(query)` - Search internal docs

#### MCP Protocol Implementation

**MCP Architecture**:
- JSON-RPC 2.0 based communication
- Tools exposed as MCP resources
- LLM function calling via MCP protocol
- stdio transport for local tools
- Standards-compliant schema definitions

```cpp
// MCP Resource (Tool) Definition
struct McpTool
{
    std::string name;
    std::string description;
    nlohmann::json input_schema;  // JSON Schema for parameters
    std::function<Result<nlohmann::json>(const nlohmann::json&)> handler;
};

// MCP Server
class McpServer
{
public:
    McpServer();
    
    // Tool registration
    void register_tool(const McpTool& tool);
    std::vector<std::string> list_tools();
    
    // MCP Protocol handlers
    Result<nlohmann::json> handle_initialize(const nlohmann::json& params);
    Result<nlohmann::json> handle_list_tools(const nlohmann::json& params);
    Result<nlohmann::json> handle_call_tool(const nlohmann::json& params);
    
    // JSON-RPC 2.0 message handling
    Result<nlohmann::json> handle_request(const nlohmann::json& request);
    
    // Get tools in LLM function calling format
    std::vector<nlohmann::json> get_function_schemas();
    
private:
    std::map<std::string, McpTool> tools_;
    std::string server_name_;
    std::string server_version_;
};

// MCP Client (for agents to call tools)
class McpClient
{
public:
    McpClient(std::shared_ptr<McpServer> server);
    
    Result<nlohmann::json> call_tool(
        const std::string& tool_name,
        const nlohmann::json& arguments
    );
    
    std::vector<nlohmann::json> list_available_tools();
};
```

---

### ✅ Phase 3: Agent System (COMPLETED)
**Status**: Complete and tested
**Goal**: Create intelligent agents with chat and tool execution capabilities

**Deliverables**:
- `include/agent.hpp` - Base agent class ✓
- `include/agents.hpp` - Specialized agent implementations ✓
- `tests/agent_tests.cpp` - Comprehensive agent tests ✓
- `examples/agent_demo.cpp` - Agent system demonstration ✓

**Implemented Features**:
- Base Agent class with automatic tool calling loop
- 6 specialized agents (CodeAssistant, DevOpsAgent, DocumentationAgent, TestingAgent, DataAnalystAgent, SecurityAgent)
- Conversation history management
- Function schema generation for LLMs
- AgentFactory for custom agent creation
- Iteration limits to prevent infinite loops
- AgentResponse with tracking
- Custom tool registration

**Specialized Agents**:
1. **CodeAssistant** (temp=0.3) - Code analysis, review, refactoring
2. **DevOpsAgent** (temp=0.2) - Infrastructure, deployment, monitoring
3. **DocumentationAgent** (temp=0.4) - Technical writing, API docs
4. **TestingAgent** (temp=0.3) - Test generation, coverage analysis
5. **DataAnalystAgent** (temp=0.4) - Data analysis, insights
6. **SecurityAgent** (temp=0.2) - Security analysis, vulnerability detection

---

### 🔄 Phase 4: Coordinator System
**Goal**: Create multi-agent coordination for complex workflows

**Deliverables**:
- `include/agent.hpp` - Base agent class and implementations
- `tests/agent_tests.cpp` - Agent behavior tests

**Design**:

#### Base Agent Class

```cpp
class Agent
{
public:
    Agent(
        const std::string& name,
        const std::string& role,
        std::shared_ptr<LlmClient> llm_client,
        std::shared_ptr<ToolRegistry> tools
    );
    
    // Core capabilities
    Result<AgentResponse> chat(const std::string& message);
    Result<AgentResponse> chat_stream(const std::string& message);
    
    // Tool execution
    Result<ToolResult> execute_tool(const std::string& tool_name, const nlohmann::json& args);
    
    // Context management
    void add_context(const std::string& context);
    void clear_context();
    std::vector<ChatMessage> get_conversation_history();
    
    // Configuration
    void set_system_prompt(const std::string& prompt);
    void set_temperature(float temp);
    void enable_tools(const std::vector<std::string>& tool_names);
    
private:
    std::string name_;
    std::string role_;
    std::string system_prompt_;
    std::vector<ChatMessage> conversation_history_;
    std::shared_ptr<LlmClient> llm_client_;
    std::shared_ptr<ToolRegistry> tools_;
    std::vector<std::string> enabled_tools_;
    CompletionConfig config_;
};

struct AgentResponse
{
    std::string message;
    std::vector<ToolCall> tool_calls;
    bool needs_tool_execution;
};

struct ToolCall
{
    std::string id;
    std::string tool_name;
    nlohmann::json arguments;
};
```

#### Specialized Agent Types

**1. Code Assistant Agent**
```cpp
class CodeAssistant : public Agent
{
    // Specialized for: code review, refactoring suggestions, bug finding
    // Tools: analyze_code, find_patterns, suggest_improvements
};
```

**2. DevOps Agent**
```cpp
class DevOpsAgent : public Agent
{
    // Specialized for: deployments, monitoring, infrastructure
    // Tools: check_services, deploy_app, restart_service, check_logs
};
```

**3. Documentation Agent**
```cpp
class DocumentationAgent : public Agent
{
    // Specialized for: writing docs, generating API references
    // Tools: generate_docs, check_spelling, format_markdown
};
```

**4. Testing Agent**
```cpp
class TestingAgent : public Agent
{
    // Specialized for: test generation, coverage analysis
    // Tools: generate_tests, run_coverage, analyze_failures
};
```

**5. General Assistant Agent**
```cpp
class GeneralAssistant : public Agent
{
    // Specialized for: general queries, task routing
    // Tools: all general-purpose tools
};
```

---

### 🔄 Phase 4: Coordinator System
**Goal**: Multi-agent orchestration and intelligent request routing

**Deliverables**:
- `include/coordinator.hpp` - Coordinator implementation
- `tests/coordinator_tests.cpp` - Multi-agent collaboration tests

**Design**:

#### Coordinator Class

```cpp
class Coordinator
{
public:
    Coordinator(std::shared_ptr<LlmClient> llm_client);
    
    // Agent management
    void register_agent(std::shared_ptr<Agent> agent);
    void remove_agent(const std::string& agent_name);
    std::vector<std::string> list_agents();
    
    // Request routing
    Result<CoordinatorResponse> handle_request(const UserRequest& request);
    Result<CoordinatorResponse> handle_request_stream(const UserRequest& request);
    
    // Multi-agent collaboration
    Result<CoordinatorResponse> collaborate(
        const std::string& task,
        const std::vector<std::string>& agent_names
    );
    
    // Session management
    void create_session(const std::string& session_id);
    void end_session(const std::string& session_id);
    
private:
    Result<std::string> route_to_agent(const UserRequest& request);
    Result<std::vector<std::string>> decompose_task(const std::string& task);
    void aggregate_results(const std::vector<AgentResponse>& responses);
    
    std::shared_ptr<LlmClient> llm_client_;
    std::map<std::string, std::shared_ptr<Agent>> agents_;
    std::map<std::string, Session> sessions_;
};

struct UserRequest
{
    std::string user_id;
    std::string session_id;
    std::string message;
    std::optional<std::string> preferred_agent;
    nlohmann::json metadata;
};

struct CoordinatorResponse
{
    std::string agent_name;
    std::string response;
    std::vector<ToolResult> tool_results;
    bool requires_followup;
    std::optional<std::string> next_agent_suggestion;
};

struct Session
{
    std::string session_id;
    std::string user_id;
    std::vector<UserRequest> request_history;
    std::map<std::string, std::string> context;
    std::chrono::system_clock::time_point created_at;
};
```

#### Routing Strategies

**1. Intent-Based Routing**
- Analyze user message intent
- Route to specialized agent based on intent classification
- Examples:
  - "Review this code" → CodeAssistant
  - "Deploy to staging" → DevOpsAgent
  - "Generate API docs" → DocumentationAgent

**2. Keyword-Based Routing**
- Match keywords to agent specializations
- Fast fallback when intent unclear

**3. Multi-Agent Collaboration**
- Break complex tasks into subtasks
- Assign subtasks to multiple agents
- Aggregate and synthesize results

**4. Learning-Based Routing**
- Track success rates per agent per task type
- Improve routing over time

---

## Phase 5: Advanced Features (Future)

### Context Management
- Long-term memory with vector database
- RAG (Retrieval Augmented Generation)
- Code repository indexing
- Conversation persistence

### Security & Access Control
- User authentication
- Role-based tool access
- Audit logging
- Sandboxed tool execution

### Performance Optimization
- Response caching
- Batch tool execution
- Connection pooling
- Async tool execution

### Integration APIs
- REST API server
- WebSocket for real-time chat
- CLI interface
- IDE plugins (VSCode, Vim, etc.)

### Monitoring & Observability
- Request/response logging
- Performance metrics
- Tool usage analytics
- Error tracking

---

## Example Use Cases

### Use Case 1: Code Review Request
**User**: "Can you review the authentication code in src/auth.cpp?"

**Flow**:
1. Coordinator receives request
2. Routes to CodeAssistant agent
3. CodeAssistant calls tools:
   - `read_file("src/auth.cpp")`
   - `analyze_code("src/auth.cpp")`
   - `find_vulnerabilities("src/auth.cpp")`
4. CodeAssistant generates review with LLM
5. Returns formatted review with suggestions

### Use Case 2: Bug Investigation
**User**: "The login feature is broken in staging. Can you help?"

**Flow**:
1. Coordinator receives request
2. Routes to DevOpsAgent
3. DevOpsAgent calls tools:
   - `check_service_status("staging")`
   - `get_recent_logs("auth-service")`
   - `check_recent_deployments()`
4. DevOpsAgent analyzes logs with LLM
5. If code issue detected, collaborates with CodeAssistant
6. Returns diagnosis and suggested fix

### Use Case 3: Documentation Generation
**User**: "Generate API documentation for the user service"

**Flow**:
1. Coordinator receives request
2. Routes to DocumentationAgent
3. DocumentationAgent calls tools:
   - `list_directory("src/services/user")`
   - `analyze_code("src/services/user/api.cpp")`
   - `extract_functions("src/services/user")`
4. DocumentationAgent generates docs with LLM
5. Calls `write_file("docs/user_api.md", content)`
6. Returns confirmation and doc preview

### Use Case 4: Multi-Agent Collaboration
**User**: "Prepare the authentication module for production release"

**Flow**:
1. Coordinator decomposes task:
   - Code review (CodeAssistant)
   - Test coverage check (TestingAgent)
   - Documentation check (DocumentationAgent)
   - Deployment readiness (DevOpsAgent)
2. Executes agents in parallel
3. Aggregates results
4. Generates release checklist
5. Returns comprehensive report

---

## Implementation Strategy

### Phase 2: MCP-Based Tools (Next)
**Timeline**: 3-4 days
**Priority**: HIGH

**Tasks**:
1. Implement MCP protocol types (mcp_types.hpp)
2. Create MCP server with JSON-RPC 2.0 handler
3. Implement tool registration system
4. Create JSON Schema validation for tool parameters
5. Implement core file system tools (MCP-compliant)
6. Implement Git tools (MCP-compliant)
7. Create MCP client for tool invocation
8. Write comprehensive MCP protocol tests
9. Document MCP tool creation guide
10. Generate LLM function calling schemas from MCP tools

### Phase 3: Agents
**Timeline**: 3-4 days
**Priority**: HIGH

**Tasks**:
1. Implement base Agent class
2. Add conversation history management
3. Implement tool calling logic (function calling)
4. Create specialized agent types
5. Add streaming support for agents
6. Write agent tests
7. Create agent configuration system

### Phase 4: Coordinator
**Timeline**: 3-4 days
**Priority**: HIGH

**Tasks**:
1. Implement Coordinator class
2. Design routing algorithm
3. Add session management
4. Implement multi-agent collaboration
5. Add context sharing between agents
6. Write coordinator tests
7. Create end-to-end integration tests

---

## Success Metrics

### Performance
- Response time < 500ms for tool execution
- LLM response time < 5s for chat
- Streaming latency < 100ms per chunk

### Reliability
- 99.9% uptime for production deployment
- Zero crashes in normal operation
- Graceful degradation on LLM errors

### Usability
- Developer can add new tool in < 30 minutes
- New agent creation in < 1 hour
- Clear error messages for all failures

### Extensibility
- Plugin system for custom tools
- Easy integration with company systems
- Support for custom LLM backends

---

## Technology Stack

- **Language**: C++23
- **Build**: CMake
- **HTTP**: cpp-httplib (header-only)
- **JSON**: nlohmann/json (header-only)
- **LLM Backend**: llama.cpp (inference engine)
- **Testing**: Custom test framework (no external dependencies)
- **Deployment**: Linux (Ubuntu 20.04+)

---

## Design Decisions Log

### Why Header-Only?
- Easy deployment to developer machines
- No linking issues
- Fast compilation with proper includes
- Simple CI/CD

### Why llama.cpp?
- On-premise deployment (data privacy)
- No API costs
- Full control over model selection
- High performance (CPU + GPU)

### Why C++23?
- Best performance for production systems
- Modern features (coroutines, concepts, ranges)
- Direct control over resources
- Team expertise

### Why Linux-Only?
- Target deployment environment
- Better process control (fork/exec)
- Superior performance
- All dev infrastructure runs on Linux

---

## Next Steps

1. **Review this roadmap** with team
2. **Start Phase 2**: Implement tools.hpp
3. **Create tool documentation** for contributors
4. **Design function calling format** compatible with llama.cpp

---

**Last Updated**: December 19, 2025  
**Version**: 1.0  
**Status**: Phase 1 Complete, Phase 2 Ready to Start
