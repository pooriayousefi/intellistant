# Intellistant User Manual

**Version:** 1.0  
**Date:** December 19, 2025  
**Author:** Pooria Yousefi

---

## Table of Contents

1. [Introduction](#introduction)
2. [System Requirements](#system-requirements)
3. [Installation](#installation)
4. [Quick Start](#quick-start)
5. [Testing the Framework](#testing-the-framework)
6. [Using the CLI](#using-the-cli)
7. [Using the REST API](#using-the-rest-api)
8. [Architecture Overview](#architecture-overview)
9. [Troubleshooting](#troubleshooting)
10. [Advanced Usage](#advanced-usage)

---

## Introduction

**Intellistant** is a multi-agent development assistant framework built in C++23. It provides intelligent routing to specialized AI agents, automated tool calling, and collaborative workflows for software development tasks.

### Key Features

- **6 Specialized Agents** - Code, DevOps, Documentation, Testing, Data Analysis, Security
- **MCP Protocol** - 12 tools for file system, Git, and system operations
- **Multiple Interfaces** - CLI, REST API, and programmatic access
- **Intelligent Routing** - Intent-based, keyword-based, and custom strategies
- **Multi-Agent Collaboration** - Coordinate multiple agents for complex tasks
- **Session Management** - Track conversation history and context
- **Monitoring** - Request logging, performance metrics, and usage statistics

---

## System Requirements

### Minimum Requirements

- **OS:** Linux (Ubuntu 20.04+ recommended)
- **CPU:** 2 cores
- **RAM:** 4 GB
- **Disk:** 10 GB free space
- **Compiler:** GCC 14+ or Clang 16+ (C++23 support)
- **CMake:** 3.20+

### Recommended Requirements

- **OS:** Linux (Ubuntu 22.04+)
- **CPU:** 4+ cores
- **RAM:** 8+ GB
- **Disk:** 20+ GB free space
- **GPU:** Optional (for faster LLM inference)

### Dependencies

- **llama.cpp server** - For LLM inference
- **cpp-httplib** - HTTP client/server (header-only, included)
- **nlohmann/json** - JSON parser (header-only, included)
- **pthreads** - Threading support (system library)

---

## Installation

### Step 1: Clone Repository

```bash
git clone https://github.com/pooriayousefi/intellistant.git
cd intellistant
```

### Step 2: Install llama.cpp Server

Intellistant requires **llama.cpp server** for LLM inference. Choose one of the following methods:

#### Method 1: Download Pre-built Binaries (Easiest)

```bash
# 1. Download pre-built release from GitHub
cd /home/pooria-yousefi/github.com/pooriayousefi/intellistant
mkdir -p runtime
cd runtime

# 2. Download latest release for your platform
# Visit: https://github.com/ggerganov/llama.cpp/releases
# Example for Linux x64:
wget https://github.com/ggerganov/llama.cpp/releases/download/b3123/llama-b3123-bin-ubuntu-x64.zip
unzip llama-b3123-bin-ubuntu-x64.zip

# 3. Copy only necessary files to runtime directory
# Required executable:
cp llama-server ./
# Or if in subdirectory:
# cp bin/llama-server ./

# Required shared libraries (if present):
cp libllama.so* ./ 2>/dev/null || true
cp libggml*.so* ./ 2>/dev/null || true

# 4. Make executable
chmod +x llama-server

# 5. Verify installation
./llama-server --version
```

**Expected output**: `llama-server version b1234 (1234) built with ...`

**Note**: Pre-built binaries may not include GPU acceleration. Use Method 2 for GPU support.

#### Method 2: Build from Source (Recommended for GPU support)

```bash
# 1. Clone llama.cpp repository
cd ~
git clone https://github.com/ggerganov/llama.cpp.git
cd llama.cpp

# 2. Build with CMake
mkdir build
cd build

# Basic build (CPU only)
cmake .. -DLLAMA_CURL=ON
cmake --build . --config Release -j$(nproc)

# 3. Copy only necessary files to Intellistant runtime directory
cd /home/pooria-yousefi/github.com/pooriayousefi/intellistant
mkdir -p runtime

# Required executable (main file needed):
cp ~/llama.cpp/build/bin/llama-server ./runtime/

# Required shared libraries only:
cp ~/llama.cpp/build/src/libllama.so ./runtime/
cp ~/llama.cpp/build/ggml/src/libggml.so ./runtime/
cp ~/llama.cpp/build/ggml/src/libggml-base.so ./runtime/
cp ~/llama.cpp/build/ggml/src/libggml-cpu.so ./runtime/

# Optional: If you built with BLAS support
cp ~/llama.cpp/build/ggml/src/libggml-blas.so ./runtime/ 2>/dev/null || true

# 4. Make executable
chmod +x ./runtime/llama-server

# 5. Verify installation
./runtime/llama-server --version
```

**Expected output**: `llama-server version b1234 (1234) built with ...`

#### GPU Acceleration (Optional - Method 2 only)

**For NVIDIA GPUs (CUDA)**:
```bash
cd ~/llama.cpp/build
cmake .. -DLLAMA_CUDA=ON -DCMAKE_CUDA_ARCHITECTURES=native -DLLAMA_CURL=ON
cmake --build . --config Release -j$(nproc)

# Copy files as in Method 2 above, plus CUDA libraries if needed
```

**For AMD GPUs (ROCm/HIP)**:
```bash
cd ~/llama.cpp/build
cmake .. -DLLAMA_HIPBLAS=ON -DLLAMA_CURL=ON
cmake --build . --config Release -j$(nproc)
```

**For Apple Silicon (Metal)**:
```bash
cd ~/llama.cpp/build
cmake .. -DLLAMA_METAL=ON -DLLAMA_CURL=ON
cmake --build . --config Release -j$(nproc)
```

#### Setting Library Path

After copying libraries, set the library path so the executable can find them:

```bash
# Option 1: Set for current session
export LD_LIBRARY_PATH=/home/pooria-yousefi/github.com/pooriayousefi/intellistant/runtime:$LD_LIBRARY_PATH

# Option 2: Add to ~/.bashrc (permanent)
echo 'export LD_LIBRARY_PATH=/home/pooria-yousefi/github.com/pooriayousefi/intellistant/runtime:$LD_LIBRARY_PATH' >> ~/.bashrc
source ~/.bashrc
```

### Step 3: Download Models

Place your LLM models in `models/` directory:

```bash
intellistant/
  models/
    qwen2.5-coder-3b/
      instruct-q4_k_m.gguf
    mistralai/
      Ministral-3-3B-Instruct-2512-Q5_K_M.gguf
```

### Step 4: Build Intellistant

```bash
cd intellistant
mkdir -p build
cd build
cmake ..
make -j4
```

### Step 5: Verify Build

Check that all executables were created:

```bash
ls -la build/

# Should see:
# intellistant_cli
# intellistant_server
# coordinator_tests
# coordinator_demo
# agent_tests
# agent_demo
# mcp_tools_tests
# llm_client_tests
```

---

## Quick Start

### 1. Start LLM Server

```bash
cd runtime
./llama-server \
  --model ../models/qwen2.5-coder-3b/instruct-q4_k_m.gguf \
  --ctx-size 8192 \
  --n-gpu-layers 0 \
  --port 8080
```

Wait for:
```
llama server listening at http://127.0.0.1:8080
```

### 2. Run Tests (Optional)

Verify everything works:

```bash
cd build
./llm_client_tests
./mcp_tools_tests
./agent_tests
./coordinator_tests
```

### 3. Try CLI

```bash
cd build
./intellistant_cli
```

### 4. Try API Server

```bash
cd build
./intellistant_server --port 8000
```

---

## Testing the Framework

### Complete Test Suite

Run all tests to verify installation:

```bash
cd /path/to/intellistant/build
```

#### Test 1: LLM Client Tests (10 tests)

```bash
./llm_client_tests
```

**Expected Output:**
```
========================================
LLM CLIENT TEST SUITE
========================================

TEST: Health Check
[✓] LLM server is healthy

TEST: Simple Completion
[✓] Completion response received
...

========================================
ALL TESTS PASSED! ✓
========================================
```

**Tests:**
1. Health check
2. Simple completion
3. Chat completion
4. System message handling
5. Multi-turn conversation
6. Temperature control
7. Max tokens limiting
8. Top-p sampling
9. Seed-based determinism
10. Tool calling integration

#### Test 2: MCP Tools Tests (9 tests)

```bash
./mcp_tools_tests
```

**Expected Output:**
```
========================================
MCP TOOLS TEST SUITE
========================================

TEST: Tool Registration
[✓] 12 tools registered

TEST: File Operations
[✓] File created: /tmp/test_file.txt
...

========================================
ALL TESTS PASSED! ✓
========================================
```

**Tests:**
1. Tool registration (12 tools)
2. File read/write operations
3. Directory listing
4. File search
5. Git status
6. Git log
7. Git diff
8. Shell command execution
9. JSON-RPC protocol compliance

#### Test 3: Agent Tests (8 tests)

```bash
./agent_tests
```

**Expected Output:**
```
========================================
AGENT SYSTEM TEST SUITE
========================================

TEST: Agent Creation
[✓] Agent created successfully

TEST: Tool Calling
[✓] Agent called read_file tool
...

========================================
ALL TESTS PASSED! ✓
========================================
```

**Tests:**
1. Agent creation and configuration
2. Simple message processing
3. Automatic tool calling
4. Multi-turn conversation
5. Tool call parsing
6. Conversation history management
7. Specialized agents (6 types)
8. Agent factory pattern

#### Test 4: Coordinator Tests (10 tests)

```bash
./coordinator_tests
```

**Expected Output:**
```
========================================
COORDINATOR SYSTEM TEST SUITE
========================================

TEST: Coordinator Creation and Setup
[✓] 6 default agents registered
...

========================================
ALL TESTS PASSED! ✓
========================================
```

**Tests:**
1. Coordinator creation
2. Agent management (register/remove/list)
3. Session management (create/end/update)
4. Keyword-based routing
5. Preferred agent routing
6. Round-robin routing
7. Multi-agent collaboration
8. Request/response structures
9. Usage statistics tracking
10. Error handling

### Demo Applications

#### Agent Demo

```bash
./agent_demo
```

Shows individual agent capabilities.

#### Coordinator Demo

```bash
./coordinator_demo
```

Demonstrates:
- Request routing
- Session management
- Multi-agent collaboration
- Routing strategies
- Real-world scenarios

### Manual Testing Checklist

#### ✅ Phase 1: LLM Client

- [ ] Connect to LLM server
- [ ] Send completion request
- [ ] Send chat request
- [ ] Verify JSON parsing
- [ ] Check error handling

#### ✅ Phase 2: MCP Tools

- [ ] List all 12 tools
- [ ] Read a file
- [ ] Write a file
- [ ] List directory
- [ ] Search files
- [ ] Git status
- [ ] Execute shell command

#### ✅ Phase 3: Agents

- [ ] Create CodeAssistant agent
- [ ] Send code review request
- [ ] Verify tool was called
- [ ] Check conversation history
- [ ] Test all 6 specialized agents

#### ✅ Phase 4: Coordinator

- [ ] List available agents
- [ ] Create session
- [ ] Send message (intent routing)
- [ ] Try keyword routing
- [ ] Multi-agent collaboration
- [ ] Check statistics

#### ✅ Phase 5: Integration

- [ ] Start CLI interface
- [ ] Start API server
- [ ] Test REST endpoints
- [ ] Check metrics
- [ ] View logs

---

## Using the CLI

### Starting the CLI

```bash
./intellistant_cli [options]

Options:
  --llm-server <url>  LLM server URL (default: localhost:8080)
  --help, -h          Show help message
```

### CLI Commands

| Command | Description | Example |
|---------|-------------|---------|
| `/help` | Show help message | `/help` |
| `/agents` | List all agents | `/agents` |
| `/stats` | Show usage statistics | `/stats` |
| `/session` | Show current session info | `/session` |
| `/agent <name>` | Set preferred agent | `/agent CodeAssistant` |
| `/routing <strategy>` | Set routing strategy | `/routing keyword` |
| `/context <key> <value>` | Set session context | `/context project intellistant` |
| `/collaborate <task> \| <agents>` | Multi-agent collaboration | `/collaborate Review code \| CodeAssistant,TestingAgent` |
| `/clear` | Clear conversation history | `/clear` |
| `/quit` | Exit CLI | `/quit` |

### Example CLI Session

```bash
$ ./intellistant_cli

██╗███╗   ██╗████████╗███████╗██╗     ██╗     ██╗███████╗████████╗ █████╗ ███╗   ██╗████████╗
...

🔧 Initializing coordinator...
✓ Loaded 6 specialized agents
✓ Connected to LLM server: localhost:8080

Type /help for available commands, or just chat with the agents!

You> /agents

🤖 Available Agents (6):

  • CodeAssistant
  • DataAnalystAgent
  • DevOpsAgent
  • DocumentationAgent
  • SecurityAgent
  • TestingAgent

You> Can you review the authentication code in src/auth.cpp?

╭─ Response from: CodeAssistant
├─ Tools used: read_file
├─ Response:
│
│  I've reviewed src/auth.cpp. Here are my findings:
│
│  1. **Security Issue**: Password comparison uses == instead of constant-time comparison
│     Line 45: if (password == stored_password)
│     Recommendation: Use crypto_compare() for timing-attack resistance
│
│  2. **Memory Leak**: Token allocation not freed
│     Line 78: char* token = malloc(256);
│     Recommendation: Add free(token) or use smart pointers
│
│  3. **Best Practice**: Missing input validation
│     Line 23: No check for null username
│     Recommendation: Add validation at function entry
│
╰─

You> /quit

👋 Goodbye!
```

---

## Using the REST API

### Starting the API Server

```bash
./intellistant_server [options]

Options:
  --port <port>       Server port (default: 8000)
  --host <host>       Server host (default: 0.0.0.0)
  --llm-server <url>  LLM server URL (default: localhost:8080)
  --routing <strategy> Routing strategy (default: intent)
  --help, -h          Show help
```

### API Endpoints

#### Health Check

```bash
GET /health
```

**Response:**
```json
{
  "status": "ok",
  "service": "intellistant-api"
}
```

#### List Agents

```bash
GET /api/agents
```

**Response:**
```json
{
  "success": true,
  "data": {
    "agents": ["CodeAssistant", "DevOpsAgent", ...],
    "count": 6
  }
}
```

#### Create Session

```bash
POST /api/sessions
Content-Type: application/json

{
  "user_id": "developer_1"
}
```

**Response:**
```json
{
  "success": true,
  "data": {
    "session_id": "req_1703001234567",
    "user_id": "developer_1"
  }
}
```

#### Send Message

```bash
POST /api/chat
Content-Type: application/json

{
  "user_id": "developer_1",
  "session_id": "req_1703001234567",
  "message": "Review src/auth.cpp for security issues",
  "preferred_agent": "SecurityAgent",
  "metadata": {
    "project": "myapp",
    "language": "cpp"
  }
}
```

**Response:**
```json
{
  "success": true,
  "data": {
    "agent": "SecurityAgent",
    "response": "Found 3 security issues...",
    "tool_results": ["read_file", "analyze_code"],
    "requires_followup": false,
    "agents_used": 1
  }
}
```

#### Multi-Agent Collaboration

```bash
POST /api/collaborate
Content-Type: application/json

{
  "task": "Prepare authentication module for production",
  "agents": ["CodeAssistant", "TestingAgent", "SecurityAgent"]
}
```

**Response:**
```json
{
  "success": true,
  "data": {
    "agent": "Collaboration",
    "response": "Production readiness report:\n\n1. Code Quality (CodeAssistant): ✓ Passes\n2. Test Coverage (TestingAgent): ✓ 85%\n3. Security Audit (SecurityAgent): ⚠ 2 issues found",
    "tool_results": ["read_file", "run_tests", "scan_vulnerabilities"],
    "agents_used": 3
  }
}
```

#### Get Metrics

```bash
GET /api/metrics
```

**Response:**
```json
{
  "success": true,
  "data": {
    "/api/chat": {
      "total_requests": 42,
      "successful_requests": 40,
      "failed_requests": 2,
      "average_duration_ms": 1250.5,
      "min_duration_ms": 450,
      "max_duration_ms": 3200
    },
    ...
  }
}
```

#### Get Logs

```bash
GET /api/logs?limit=100
```

**Response:**
```json
{
  "success": true,
  "data": [
    {
      "request_id": "req_1703001234567",
      "endpoint": "/api/chat",
      "method": "POST",
      "user_id": "developer_1",
      "duration_ms": 1250,
      "status_code": 200,
      "request_size": 256,
      "response_size": 1024
    },
    ...
  ]
}
```

#### Get Statistics

```bash
GET /api/stats
```

**Response:**
```json
{
  "success": true,
  "data": {
    "agent_usage": {
      "CodeAssistant": 45,
      "DevOpsAgent": 23,
      "SecurityAgent": 12,
      ...
    },
    "active_sessions": 5
  }
}
```

### Example: Using curl

```bash
# Health check
curl http://localhost:8000/health

# List agents
curl http://localhost:8000/api/agents

# Create session
curl -X POST http://localhost:8000/api/sessions \
  -H "Content-Type: application/json" \
  -d '{"user_id": "developer_1"}'

# Send message
curl -X POST http://localhost:8000/api/chat \
  -H "Content-Type: application/json" \
  -d '{
    "user_id": "developer_1",
    "session_id": "req_1703001234567",
    "message": "Review auth.cpp"
  }'

# Collaboration
curl -X POST http://localhost:8000/api/collaborate \
  -H "Content-Type: application/json" \
  -d '{
    "task": "Prepare for production",
    "agents": ["CodeAssistant", "TestingAgent", "SecurityAgent"]
  }'

# Get metrics
curl http://localhost:8000/api/metrics

# Get logs
curl http://localhost:8000/api/logs?limit=50

# Get stats
curl http://localhost:8000/api/stats
```

---

## Architecture Overview

### System Components

```
┌─────────────────────────────────────────────────────────┐
│                    Intellistant Framework               │
├─────────────────────────────────────────────────────────┤
│                                                         │
│  ┌───────────────────────────────────────────────────┐ │
│  │            User Interfaces                        │ │
│  ├───────────────────────────────────────────────────┤ │
│  │  • CLI (intellistant_cli)                         │ │
│  │  • REST API (intellistant_server)                 │ │
│  │  • Programmatic (C++ library)                     │ │
│  └───────────────────────────────────────────────────┘ │
│                         ↓                               │
│  ┌───────────────────────────────────────────────────┐ │
│  │         Coordinator (Phase 4)                     │ │
│  ├───────────────────────────────────────────────────┤ │
│  │  • Request routing (4 strategies)                 │ │
│  │  • Session management                             │ │
│  │  • Multi-agent collaboration                      │ │
│  │  • Statistics & monitoring                        │ │
│  └───────────────────────────────────────────────────┘ │
│                         ↓                               │
│  ┌───────────────────────────────────────────────────┐ │
│  │         Specialized Agents (Phase 3)              │ │
│  ├───────────────────────────────────────────────────┤ │
│  │  • CodeAssistant      • TestingAgent              │ │
│  │  • DevOpsAgent        • DataAnalystAgent          │ │
│  │  • DocumentationAgent • SecurityAgent             │ │
│  └───────────────────────────────────────────────────┘ │
│                         ↓                               │
│  ┌───────────────────────────────────────────────────┐ │
│  │         MCP Tools (Phase 2)                       │ │
│  ├───────────────────────────────────────────────────┤ │
│  │  • File system (7 tools)                          │ │
│  │  • Git operations (4 tools)                       │ │
│  │  • System commands (1 tool)                       │ │
│  └───────────────────────────────────────────────────┘ │
│                         ↓                               │
│  ┌───────────────────────────────────────────────────┐ │
│  │         LLM Client (Phase 1)                      │ │
│  ├───────────────────────────────────────────────────┤ │
│  │  • llama.cpp integration                          │ │
│  │  • Chat completions                               │ │
│  │  • Tool calling support                           │ │
│  └───────────────────────────────────────────────────┘ │
│                                                         │
└─────────────────────────────────────────────────────────┘
```

### Data Flow

1. **User Input** → CLI or REST API
2. **Coordinator** → Routes to appropriate agent
3. **Agent** → Processes message with LLM
4. **LLM** → Returns response with tool calls
5. **Agent** → Executes tools via MCP
6. **MCP** → Performs file/git/system operations
7. **Agent** → Sends results back to LLM
8. **LLM** → Generates final response
9. **Coordinator** → Returns to user

---

## Troubleshooting

### Common Issues

#### 1. "Failed to connect to LLM server"

**Problem:** LLM server not running or wrong URL

**Solution:**
```bash
# Check if llama-server is running
ps aux | grep llama-server

# Start llama-server
cd runtime
./llama-server --model ../models/qwen2.5-coder-3b/instruct-q4_k_m.gguf --port 8080

# Test connection
curl http://localhost:8080/health
# Should return: {"status":"ok"}
```

#### 2. "llama-server: command not found"

**Problem:** llama-server not in runtime/ directory or not executable

**Solution:**
```bash
# Check if llama-server exists
ls -la runtime/llama-server

# If missing, rebuild llama.cpp (see Step 2 of Installation)
cd ~/llama.cpp/build
cmake --build . --config Release -j$(nproc)
cp bin/llama-server /path/to/intellistant/runtime/

# Make executable
chmod +x runtime/llama-server
```

#### 3. "libllama.so: cannot open shared object file"

**Problem:** Missing shared libraries or incorrect library path

**Solution:**
```bash
# Ensure all required libraries are in runtime directory:
ls -la runtime/libllama.so
ls -la runtime/libggml*.so

# If missing, copy from llama.cpp build:
cp ~/llama.cpp/build/src/libllama.so runtime/
cp ~/llama.cpp/build/ggml/src/libggml.so runtime/
cp ~/llama.cpp/build/ggml/src/libggml-base.so runtime/
cp ~/llama.cpp/build/ggml/src/libggml-cpu.so runtime/

# Set library path
export LD_LIBRARY_PATH=$PWD/runtime:$LD_LIBRARY_PATH

# Or add to ~/.bashrc permanently
echo 'export LD_LIBRARY_PATH=/home/pooria-yousefi/github.com/pooriayousefi/intellistant/runtime:$LD_LIBRARY_PATH' >> ~/.bashrc
source ~/.bashrc
```

#### 4. "Model file not found"

**Problem:** Model path incorrect or file doesn't exist

**Solution:**
```bash
# Check model exists
ls -lh models/qwen2.5-coder-3b/instruct-q4_k_m.gguf

# Download model if missing (example with Qwen 2.5 Coder 3B)
mkdir -p models/qwen2.5-coder-3b
cd models/qwen2.5-coder-3b

# Use huggingface-cli or wget
huggingface-cli download Qwen/Qwen2.5-Coder-3B-Instruct-GGUF \
  qwen2.5-coder-3b-instruct-q4_k_m.gguf \
  --local-dir . \
  --local-dir-use-symlinks False

# Or with wget
wget https://huggingface.co/Qwen/Qwen2.5-Coder-3B-Instruct-GGUF/resolve/main/qwen2.5-coder-3b-instruct-q4_k_m.gguf \
  -O instruct-q4_k_m.gguf
```

#### 5. "Compilation errors"

**Problem:** Missing C++23 support

**Solution:**
```bash
# Check compiler version
g++ --version  # Should be 14+
clang++ --version  # Should be 16+

# Update compiler if needed
sudo apt update
sudo apt install g++-14
```

#### 6. "Tests failing"

**Problem:** LLM server not responding

**Solution:**
```bash
# Test LLM server manually
curl http://localhost:8080/health

# Should return: {"status":"ok"}
```

#### 4. "Port already in use"

**Problem:** API server port 8000 occupied

**Solution:**
```bash
# Use different port
./intellistant_server --port 8001

# Or kill process using port 8000
lsof -ti:8000 | xargs kill -9
```

#### 5. "Tool execution failed"

**Problem:** Permission issues

**Solution:**
```bash
# Check file permissions
ls -la /path/to/file

# Fix permissions
chmod +r /path/to/file
```

### Debug Mode

Enable verbose logging:

```bash
# For CLI
./intellistant_cli --verbose

# For API server
./intellistant_server --log-level debug
```

### Getting Help

1. Check documentation in `docs/` folder
2. Review test output for error messages
3. Check LLM server logs
4. Verify all dependencies installed

---

## Advanced Usage

### Custom Agent Creation

```cpp
#include "agent.hpp"

// Create custom agent
AgentConfig config;
config.name = "DatabaseAgent";
config.system_prompt = "You are a database expert...";
config.llm_config.temperature = 0.2;

auto db_agent = std::make_shared<Agent>(config, "localhost:8080");

// Register with coordinator
coordinator.register_agent("DatabaseAgent", db_agent);
```

### Custom Routing Strategy

```cpp
// Implement custom routing logic
class CustomRouter : public Coordinator
{
    std::expected<std::string, std::string> 
    custom_route(const std::string& message)
    {
        // Your routing logic here
        if (message.contains("database"))
            return "DatabaseAgent";
        return "CodeAssistant";
    }
};
```

### Batch Processing

```cpp
// Process multiple requests
std::vector<std::string> messages = {
    "Review auth.cpp",
    "Check test coverage",
    "Update documentation"
};

for (const auto& msg : messages)
{
    UserRequest req{.message = msg};
    auto result = coordinator.handle_request(req);
    // Handle result
}
```

### Integration with CI/CD

```bash
#!/bin/bash
# ci-review.sh

# Start llama-server
./runtime/llama-server --model models/qwen2.5-coder-3b/instruct-q4_k_m.gguf --port 8080 &
LLM_PID=$!

# Wait for server
sleep 5

# Run code review
./build/intellistant_cli <<EOF
Can you review all C++ files in src/ for code quality issues?
/quit
EOF

# Cleanup
kill $LLM_PID
```

---

## Appendices

### A. File Structure

```
intellistant/
├── include/               # Header files
│   ├── llm_client.hpp    # Phase 1: LLM client
│   ├── mcp_server.hpp    # Phase 2: MCP server
│   ├── mcp_tools.hpp     # Phase 2: MCP tools
│   ├── agent.hpp         # Phase 3: Base agent
│   ├── agents.hpp        # Phase 3: Specialized agents
│   ├── coordinator.hpp   # Phase 4: Coordinator
│   └── api_server.hpp    # Phase 5: REST API
├── src/                   # Source files
│   ├── intellistant_cli.cpp
│   └── intellistant_server.cpp
├── tests/                 # Test files
│   ├── llm_client_tests.cpp
│   ├── mcp_tools_tests.cpp
│   ├── agent_tests.cpp
│   └── coordinator_tests.cpp
├── examples/              # Example applications
│   ├── agent_demo.cpp
│   ├── mcp_demo.cpp
│   └── coordinator_demo.cpp
├── docs/                  # Documentation
│   ├── README.md
│   ├── STATUS.md
│   ├── ROADMAP.md
│   ├── PHASE2_COMPLETE.md
│   ├── PHASE3_COMPLETE.md
│   ├── PHASE4_COMPLETE.md
│   ├── PHASE5_COMPLETE.md
│   ├── DOCUMENTATION.md
│   └── USER_MANUAL.md (this file)
├── models/                # LLM models
├── runtime/               # llama.cpp binaries
├── build/                 # Build output
└── CMakeLists.txt         # Build configuration
```

### B. Agent Specifications

| Agent | Temperature | Purpose | Best For |
|-------|-------------|---------|----------|
| CodeAssistant | 0.3 | Code review, refactoring | Code quality, bugs |
| DevOpsAgent | 0.2 | Deployment, CI/CD | Infrastructure, deployment |
| DocumentationAgent | 0.4 | API docs, README | Documentation tasks |
| TestingAgent | 0.3 | Unit tests, integration | Test coverage, quality |
| DataAnalystAgent | 0.4 | Metrics, analytics | Performance, data |
| SecurityAgent | 0.2 | Security audit | Vulnerabilities, auth |

### C. Tool Reference

| Tool | Category | Description |
|------|----------|-------------|
| `read_file` | Filesystem | Read file contents |
| `write_file` | Filesystem | Write to file |
| `list_directory` | Filesystem | List directory contents |
| `search_files` | Filesystem | Search for files by pattern |
| `get_file_info` | Filesystem | Get file metadata |
| `create_directory` | Filesystem | Create directory |
| `delete_path` | Filesystem | Delete file/directory |
| `git_status` | Git | Get repository status |
| `git_log` | Git | View commit history |
| `git_diff` | Git | Show changes |
| `git_branch_info` | Git | Branch information |
| `execute_command` | System | Run shell command |

### D. Configuration Options

#### LLM Client Configuration

```cpp
CompletionConfig config;
config.temperature = 0.3;        // 0.0-1.0 (randomness)
config.top_p = 0.9;              // Nucleus sampling
config.top_k = 40;               // Top-k sampling
config.max_tokens = 1024;        // Response length limit
config.repeat_penalty = 1.1;     // Repetition control
config.seed = 42;                // Deterministic output
```

#### Agent Configuration

```cpp
AgentConfig config;
config.name = "MyAgent";
config.system_prompt = "You are...";
config.llm_config.temperature = 0.3;
config.llm_config.max_tokens = 1024;
config.max_tool_calls = 10;      // Tool call limit
```

### E. Performance Benchmarks

| Operation | Average Time | Notes |
|-----------|--------------|-------|
| Simple completion | ~500ms | Without tools |
| Chat with 1 tool call | ~1.2s | Includes tool execution |
| Multi-agent collaboration (3 agents) | ~3.5s | Sequential processing |
| Keyword routing | <1ms | Pattern matching |
| Intent routing | ~200ms | LLM classification |

---

## Conclusion

You now have a complete understanding of how to:

1. ✅ Install and configure Intellistant
2. ✅ Run all tests to verify installation
3. ✅ Use the CLI for interactive development assistance
4. ✅ Use the REST API for programmatic access
5. ✅ Understand the architecture and data flow
6. ✅ Troubleshoot common issues
7. ✅ Extend the framework with custom agents

For more information, see:
- [README.md](README.md) - Project overview
- [ROADMAP.md](ROADMAP.md) - Development roadmap
- [PHASE5_COMPLETE.md](PHASE5_COMPLETE.md) - Phase 5 details
- [DOCUMENTATION.md](DOCUMENTATION.md) - Documentation index

**Happy coding with Intellistant! 🚀**
