# macOS Offline Deployment Guide

Complete guide for deploying Intellistant on macOS without internet access.

---

## Part 1: Preparation (On Machine with Internet)

### Step 1: Download Docker Desktop for Mac

**Download Link**: https://desktop.docker.com/mac/main/amd64/Docker.dmg (Intel) or https://desktop.docker.com/mac/main/arm64/Docker.dmg (Apple Silicon)

**Size**: ~600-700 MB

**Requirements**:
- macOS 11 Big Sur or later
- At least 4 GB RAM
- VirtualBox prior to version 4.3.30 must NOT be installed

**Determine Your Mac Type**:
```bash
# Check if Apple Silicon (M1/M2/M3) or Intel
uname -m

# Output:
# arm64   -> Apple Silicon (download ARM version)
# x86_64  -> Intel (download x64 version)
```

---

### Step 2: Download GGUF Model File

Choose one model (recommended: Qwen 2.5 Coder 3B):

#### Option A: Qwen 2.5 Coder 3B (Recommended)
```bash
mkdir -p transfer-files/models/qwen2.5-coder-3b
cd transfer-files/models/qwen2.5-coder-3b

curl -L https://huggingface.co/Qwen/Qwen2.5-Coder-3B-Instruct-GGUF/resolve/main/qwen2.5-coder-3b-instruct-q4_k_m.gguf \
  -o instruct-q4_k_m.gguf
```
**Size**: ~2.3 GB

#### Option B: Mistral 3B
```bash
mkdir -p transfer-files/models/mistralai
cd transfer-files/models/mistralai

curl -L https://huggingface.co/mistralai/Ministral-3B-Instruct-2410-GGUF/resolve/main/Ministral-3B-Instruct-2410.Q4_K_M.gguf \
  -o Ministral-3-3B-Instruct-2512-Q5_K_M.gguf
```
**Size**: ~2.4 GB

---

### Step 3: Prepare Docker Images

```bash
# Navigate to your working directory
mkdir -p transfer-files
cd transfer-files

# Pull llama.cpp server image
docker pull ghcr.io/ggerganov/llama.cpp:server

# Save llama.cpp image to tar file
docker save ghcr.io/ggerganov/llama.cpp:server -o llama-cpp-server.tar

# Clone Intellistant repository
git clone https://github.com/pooriayousefi/intellistant.git
cd intellistant

# Build Intellistant Docker image
docker build -t intellistant:v1.0 .

# Save Intellistant image to tar file
cd ..
docker save intellistant:v1.0 -o intellistant-v1.0.tar
```

**Result Files**:
- `llama-cpp-server.tar` (~2-3 GB)
- `intellistant-v1.0.tar` (~1-2 GB)

---

### Step 4: Package Everything for Transfer

```bash
# Create archive with all necessary files
cd transfer-files
tar -czf intellistant-macos-deployment.tar.gz \
  llama-cpp-server.tar \
  intellistant-v1.0.tar \
  intellistant/ \
  models/

# Verify archive
ls -lh intellistant-macos-deployment.tar.gz
```

---

## Part 2: Transfer Files to macOS

Transfer these files to macOS machine:
1. `Docker.dmg` (~600-700 MB) - Choose Intel or Apple Silicon version
2. `intellistant-macos-deployment.tar.gz` (~6-8 GB)

**Transfer Methods**:
- USB drive (formatted as exFAT for compatibility)
- External SSD
- AirDrop (for smaller files)
- Network file share
- Cloud storage (download before going offline)

---

## Part 3: Installation on macOS (Offline)

### Step 1: Install Docker Desktop

1. **Mount DMG**:
   ```bash
   # Double-click Docker.dmg in Finder
   # Or from terminal:
   open Docker.dmg
   ```

2. **Install Docker**:
   - Drag Docker icon to Applications folder
   - Open Applications folder
   - Launch Docker
   - Follow setup wizard
   - Grant necessary permissions when prompted

3. **Start Docker Engine**:
   - Wait for Docker icon to appear in menu bar
   - Click icon, verify "Docker Desktop is running"

4. **Verify Installation**:
   ```bash
   docker --version
   docker ps
   ```

---

### Step 2: Extract Deployment Package

```bash
# Navigate to Downloads or wherever you saved the files
cd ~/Downloads

# Extract archive
tar -xzf intellistant-macos-deployment.tar.gz

# Navigate to extracted directory
cd intellistant-macos-deployment
```

---

### Step 3: Load Docker Images

```bash
# Load llama.cpp server image
docker load -i llama-cpp-server.tar

# Load Intellistant image
docker load -i intellistant-v1.0.tar

# Verify images are loaded
docker images
```

**Expected Output**:
```
REPOSITORY                          TAG       IMAGE ID       SIZE
intellistant                        v1.0      xxxxxxxxxxxx   ~1-2GB
ghcr.io/ggerganov/llama.cpp        server    xxxxxxxxxxxx   ~2-3GB
```

---

### Step 4: Setup Intellistant

```bash
# Navigate to intellistant directory
cd intellistant

# Verify model file exists
ls -lh models/qwen2.5-coder-3b/instruct-q4_k_m.gguf
# Or for Mistral:
# ls -lh models/mistralai/Ministral-3-3B-Instruct-2512-Q5_K_M.gguf

# Check docker-compose.yml
cat docker-compose.yml
```

---

### Step 5: Start Services with Docker Compose

```bash
# Start all services in background
docker-compose up -d

# Check service status
docker-compose ps

# View logs
docker-compose logs -f
```

**Expected Services**:
- `llama-server` (port 8080)
- `intellistant-api` (port 8000)
- `intellistant-cli` (optional)

---

### Step 6: Verify Deployment

```bash
# Test LLM server health
curl http://localhost:8080/health

# Test Intellistant API
curl http://localhost:8000/health

# Expected response:
# {"status": "healthy", "timestamp": "..."}

# List available agents
curl http://localhost:8000/api/agents

# Test chat endpoint
curl -X POST http://localhost:8000/api/chat \
  -H "Content-Type: application/json" \
  -d '{
    "user_id": "test_user",
    "message": "Hello, can you help me?"
  }'
```

---

### Step 7: Use Interactive CLI

```bash
# Enter CLI container
docker exec -it intellistant-cli bash

# Inside container, run CLI
./intellistant_cli

# You should see the interactive interface:
# ╔════════════════════════════════════════════════════╗
# ║            INTELLISTANT v1.0                       ║
# ║     Multi-Agent Development Assistant CLI          ║
# ╚════════════════════════════════════════════════════╝
```

---

## Part 4: Common Operations

### Start Services
```bash
cd ~/path/to/intellistant
docker-compose up -d
```

### Stop Services
```bash
docker-compose down
```

### View Logs
```bash
# All services
docker-compose logs -f

# Specific service
docker-compose logs -f llama-server
docker-compose logs -f intellistant-api
```

### Restart Services
```bash
docker-compose restart
```

### Check Service Status
```bash
docker-compose ps
docker stats
```

---

## Part 5: Troubleshooting

### Issue 1: Docker Desktop Won't Start

**Solution**:
```bash
# Reset Docker Desktop
# Click Docker menu bar icon -> Troubleshoot -> Reset to factory defaults

# Or restart Docker from terminal
killall Docker && open /Applications/Docker.app

# Check system resources
top -l 1 | grep PhysMem
```

---

### Issue 2: "Cannot connect to Docker daemon"

**Solution**:
```bash
# Ensure Docker Desktop is running
ps aux | grep Docker

# Check Docker socket
ls -la /var/run/docker.sock

# Restart Docker Desktop
osascript -e 'quit app "Docker"'
open /Applications/Docker.app
```

---

### Issue 3: Port Already in Use

**Solution**:
```bash
# Check what's using port 8000 or 8080
lsof -i :8000
lsof -i :8080

# Kill process using the port
kill -9 <PID>

# Or change port in docker-compose.yml
# Edit ports section: "9000:8000" instead of "8000:8000"
```

---

### Issue 4: Permission Denied for Docker Socket

**Solution**:
```bash
# Add your user to docker group (if not already)
sudo dscl . -append /Groups/docker GroupMembership $(whoami)

# Or run with sudo (not recommended)
sudo docker-compose up -d
```

---

### Issue 5: Model File Not Found

**Solution**:
```bash
# Check model path
ls -la models/

# Verify docker-compose.yml volume mapping
cat docker-compose.yml | grep models

# For Qwen model, ensure:
# volumes:
#   - ./models/qwen2.5-coder-3b:/models

# Update path in docker-compose.yml if needed
```

---

### Issue 6: LLM Server Not Responding

**Solution**:
```bash
# Check llama-server logs
docker-compose logs llama-server

# Restart llama-server
docker-compose restart llama-server

# Wait for model to load (can take 30-60 seconds)
sleep 30

# Test again
curl http://localhost:8080/health

# Check memory usage (models need 4-8 GB RAM)
docker stats llama-server
```

---

### Issue 7: Slow Performance on Apple Silicon

**Solution**:
```bash
# Verify you're using ARM64 Docker images
docker image inspect ghcr.io/ggerganov/llama.cpp:server | grep Architecture

# For Apple Silicon, build llama.cpp with Metal support
cd ~/llama.cpp
mkdir build && cd build
cmake .. -DLLAMA_METAL=ON -DLLAMA_CURL=ON
cmake --build . --config Release -j$(sysctl -n hw.ncpu)

# Copy Metal-optimized binary
cp bin/llama-server ~/intellistant/runtime/
```

---

### Issue 8: Rosetta 2 Warning (Apple Silicon)

If you see warnings about Rosetta:

**Solution**:
```bash
# Install Rosetta 2 (for running Intel containers on Apple Silicon)
softwareupdate --install-rosetta --agree-to-license

# Restart Docker Desktop
osascript -e 'quit app "Docker"'
open /Applications/Docker.app
```

---

## Part 6: Performance Tuning

### Allocate More Resources to Docker

1. Open Docker Desktop
2. Click Docker icon in menu bar → Preferences → Resources
3. Adjust:
   - **CPUs**: 4-8 cores (for faster inference)
   - **Memory**: 8-16 GB (models need 4-8 GB)
   - **Swap**: 2-4 GB
   - **Disk**: 50+ GB

### Metal GPU Acceleration (Apple Silicon)

For **best performance on M1/M2/M3 Macs**, use Metal-optimized llama.cpp:

```bash
# Build llama.cpp with Metal support
git clone https://github.com/ggerganov/llama.cpp.git
cd llama.cpp
mkdir build && cd build

cmake .. -DLLAMA_METAL=ON -DLLAMA_CURL=ON
cmake --build . --config Release -j$(sysctl -n hw.ncpu)

# Copy to Intellistant runtime
cp bin/llama-server ~/path/to/intellistant/runtime/
cp src/libllama.dylib ~/path/to/intellistant/runtime/
cp ggml/src/libggml*.dylib ~/path/to/intellistant/runtime/

# Restart services
cd ~/path/to/intellistant
docker-compose restart
```

**Performance Improvement**: 2-5x faster inference with Metal

---

### Optimize for Intel Macs

```bash
# Build with AVX2 and FMA optimizations
cmake .. -DLLAMA_AVX2=ON -DLLAMA_FMA=ON -DLLAMA_CURL=ON
cmake --build . --config Release -j$(sysctl -n hw.ncpu)
```

---

## Part 7: Testing the Framework

### Test REST API

```bash
# Health check
curl http://localhost:8000/health

# List agents
curl http://localhost:8000/api/agents

# Chat with CodeAssistant
curl -X POST http://localhost:8000/api/chat \
  -H "Content-Type: application/json" \
  -d '{
    "user_id": "developer_1",
    "message": "Write a Python function to calculate factorial"
  }'

# Multi-agent collaboration
curl -X POST http://localhost:8000/api/collaborate \
  -H "Content-Type: application/json" \
  -d '{
    "user_id": "developer_1",
    "task": "Review and test a login function",
    "agents": ["CodeAssistant", "TestingAgent", "SecurityAgent"]
  }'

# Get metrics
curl http://localhost:8000/api/metrics
```

---

### Test CLI Interface

```bash
# Interactive CLI
docker exec -it intellistant-cli ./intellistant_cli

# Try commands:
You> /help
You> /agents
You> Write a bash script to backup a directory
You> /collaborate Review this code for security issues
```

---

## Part 8: Native Build (Alternative to Docker)

If you prefer running natively without Docker:

### Prerequisites

```bash
# Install Homebrew (if not already installed)
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install dependencies
brew install cmake gcc git curl

# For Apple Silicon, install Xcode Command Line Tools
xcode-select --install
```

### Build llama.cpp

```bash
# Clone and build llama.cpp
git clone https://github.com/ggerganov/llama.cpp.git
cd llama.cpp
mkdir build && cd build

# For Apple Silicon (Metal GPU)
cmake .. -DLLAMA_METAL=ON -DLLAMA_CURL=ON

# For Intel (AVX2/FMA)
cmake .. -DLLAMA_AVX2=ON -DLLAMA_FMA=ON -DLLAMA_CURL=ON

# Build
cmake --build . --config Release -j$(sysctl -n hw.ncpu)
```

### Build Intellistant

```bash
# Clone Intellistant
git clone https://github.com/pooriayousefi/intellistant.git
cd intellistant

# Copy llama.cpp binaries
mkdir -p runtime
cp ~/llama.cpp/build/bin/llama-server runtime/
cp ~/llama.cpp/build/src/libllama.dylib runtime/
cp ~/llama.cpp/build/ggml/src/libggml*.dylib runtime/

# Build Intellistant
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(sysctl -n hw.ncpu)

# Run tests
./llm_client_tests
./mcp_tools_tests
./agent_tests
./coordinator_tests
```

### Run Native

```bash
# Start llama-server
cd runtime
./llama-server --model ../models/qwen2.5-coder-3b/instruct-q4_k_m.gguf --port 8080 &

# Run API server
cd ../build
./intellistant_server --port 8000 &

# Or run CLI
./intellistant_cli
```

---

## Part 9: Updating the Framework

### Pull Latest Changes (when back online)

```bash
cd ~/path/to/intellistant
git pull origin main

# Rebuild Docker image
docker-compose down
docker build -t intellistant:v1.0 .
docker-compose up -d
```

### Update Model

```bash
# Download new model (when online)
curl -L https://huggingface.co/... -o models/new-model.gguf

# Update docker-compose.yml to point to new model
# Restart llama-server
docker-compose restart llama-server
```

---

## Part 10: Uninstallation

### Remove Docker Images

```bash
docker-compose down
docker rmi intellistant:v1.0
docker rmi ghcr.io/ggerganov/llama.cpp:server
```

### Remove Docker Desktop

```bash
# Quit Docker
osascript -e 'quit app "Docker"'

# Remove application
rm -rf /Applications/Docker.app
rm -rf ~/Library/Group\ Containers/group.com.docker
rm -rf ~/Library/Containers/com.docker.docker
rm -rf ~/.docker
```

### Remove Intellistant Files

```bash
rm -rf ~/path/to/intellistant
rm -rf ~/Downloads/intellistant-macos-deployment.tar.gz
```

---

## Summary Checklist

### Files to Download (with Internet):
- [ ] Docker Desktop for Mac (~600-700 MB) - Intel or Apple Silicon
- [ ] llama-cpp-server.tar (~2-3 GB)
- [ ] intellistant-v1.0.tar (~1-2 GB)
- [ ] GGUF model file (~2-3 GB)
- [ ] Intellistant source code (~500 KB)

### Installation Steps (Offline):
- [ ] Install Docker Desktop
- [ ] Load Docker images
- [ ] Extract Intellistant source
- [ ] Verify model files
- [ ] Start services with docker-compose
- [ ] Verify health endpoints
- [ ] Test chat API

### Verification:
- [ ] http://localhost:8080/health returns 200 OK
- [ ] http://localhost:8000/health returns healthy status
- [ ] Chat API returns responses
- [ ] CLI interface accessible

---

## File Sizes Summary

| Component | Size |
|-----------|------|
| Docker Desktop (Mac) | ~600-700 MB |
| llama-cpp-server.tar | ~2-3 GB |
| intellistant-v1.0.tar | ~1-2 GB |
| GGUF Model | ~2-3 GB |
| Intellistant Source | ~500 KB |
| **Total** | **~6-9 GB** |

---

## macOS-Specific Notes

### Apple Silicon (M1/M2/M3) Advantages:
- **Metal GPU acceleration** for 2-5x faster inference
- **Unified memory** enables larger models
- **Native ARM64** containers run efficiently
- **Lower power consumption** vs Intel

### Intel Mac Considerations:
- Use **AVX2/FMA** optimizations for best performance
- May need **more RAM** (12+ GB recommended)
- **Rosetta 2** not needed (x64 native)

### Common macOS Shortcuts:
- **⌘ + Space** → Type "Docker" to launch
- **⌘ + Q** → Quit Docker Desktop
- **⌃ + C** → Stop docker-compose logs

---

## Support

For issues, see:
- [USER_MANUAL.md](docs/USER_MANUAL.md) - Complete user guide
- [DOCKER.md](DOCKER.md) - Docker deployment guide
- [GitHub Issues](https://github.com/pooriayousefi/intellistant/issues)

---

**Last Updated**: December 19, 2025  
**Version**: v1.0.0  
**Platform**: macOS 11+ (Big Sur and later)
