# Windows 10 Offline Deployment Guide

Complete guide for deploying Intellistant on Windows 10 without internet access.

---

## Part 1: Preparation (On Machine with Internet)

### Step 1: Download Docker Desktop for Windows

**Download Link**: https://desktop.docker.com/win/main/amd64/Docker%20Desktop%20Installer.exe

**Size**: ~500 MB

**Requirements**:
- Windows 10 64-bit: Pro, Enterprise, or Education (Build 19041 or higher)
- Hardware virtualization enabled in BIOS
- WSL 2 feature enabled

---

### Step 2: Download GGUF Model File

Choose one model (recommended: Qwen 2.5 Coder 3B):

#### Option A: Qwen 2.5 Coder 3B (Recommended)
```bash
mkdir -p transfer-files/models/qwen2.5-coder-3b
cd transfer-files/models/qwen2.5-coder-3b

wget https://huggingface.co/Qwen/Qwen2.5-Coder-3B-Instruct-GGUF/resolve/main/qwen2.5-coder-3b-instruct-q4_k_m.gguf \
  -O instruct-q4_k_m.gguf
```
**Size**: ~2.3 GB

#### Option B: Mistral 3B
```bash
mkdir -p transfer-files/models/mistralai
cd transfer-files/models/mistralai

wget https://huggingface.co/mistralai/Ministral-3B-Instruct-2410-GGUF/resolve/main/Ministral-3B-Instruct-2410.Q4_K_M.gguf \
  -O Ministral-3-3B-Instruct-2512-Q5_K_M.gguf
```
**Size**: ~2.4 GB

---

### Step 3: Prepare Docker Images

```bash
# Navigate to your working directory
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
tar -czf intellistant-windows-deployment.tar.gz \
  llama-cpp-server.tar \
  intellistant-v1.0.tar \
  intellistant/ \
  models/

# Or create a zip file (if tar not available on Windows)
zip -r intellistant-windows-deployment.zip \
  llama-cpp-server.tar \
  intellistant-v1.0.tar \
  intellistant/ \
  models/
```

---

## Part 2: Transfer Files to Windows 10

Transfer these files to Windows 10 machine:
1. `Docker Desktop Installer.exe` (~500 MB)
2. `intellistant-windows-deployment.tar.gz` or `.zip` (~6-8 GB)

**Transfer Methods**:
- USB drive
- External hard drive
- Network file share
- Cloud storage (download before going offline)

---

## Part 3: Installation on Windows 10 (Offline)

### Step 1: Install Docker Desktop

1. **Run Installer**:
   ```powershell
   # Double-click: Docker Desktop Installer.exe
   # Or run from PowerShell:
   .\Docker_Desktop_Installer.exe install
   ```

2. **Enable WSL 2** (if prompted):
   - Open PowerShell as Administrator
   ```powershell
   wsl --install
   ```
   - Restart computer

3. **Start Docker Desktop**:
   - Launch "Docker Desktop" from Start Menu
   - Wait for Docker Engine to start (check system tray icon)

4. **Verify Docker Installation**:
   ```powershell
   docker --version
   docker ps
   ```

---

### Step 2: Extract Deployment Package

```powershell
# Navigate to where you saved the files
cd C:\Users\YourUsername\Downloads

# Extract using Windows built-in tar (Windows 10 1803+)
tar -xzf intellistant-windows-deployment.tar.gz

# Or if you have a .zip file
Expand-Archive intellistant-windows-deployment.zip -DestinationPath .

# Navigate to extracted directory
cd intellistant-windows-deployment
```

---

### Step 3: Load Docker Images

```powershell
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

```powershell
# Navigate to intellistant directory
cd intellistant

# Verify model file exists
dir models\qwen2.5-coder-3b\instruct-q4_k_m.gguf
# Or for Mistral:
# dir models\mistralai\Ministral-3-3B-Instruct-2512-Q5_K_M.gguf

# Update docker-compose.yml to use Windows paths (if needed)
# Edit docker-compose.yml and ensure volume paths are correct
```

---

### Step 5: Start Services with Docker Compose

```powershell
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

```powershell
# Test LLM server health
curl http://localhost:8080/health

# Test Intellistant API
curl http://localhost:8000/health

# Expected response:
# {"status": "healthy", "timestamp": "..."}

# List available agents
curl http://localhost:8000/api/agents

# Test chat endpoint
curl -X POST http://localhost:8000/api/chat `
  -H "Content-Type: application/json" `
  -d '{\"user_id\":\"test_user\",\"message\":\"Hello, can you help me?\"}'
```

---

### Step 7: Use Interactive CLI

```powershell
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
```powershell
cd C:\path\to\intellistant
docker-compose up -d
```

### Stop Services
```powershell
docker-compose down
```

### View Logs
```powershell
# All services
docker-compose logs -f

# Specific service
docker-compose logs -f llama-server
docker-compose logs -f intellistant-api
```

### Restart Services
```powershell
docker-compose restart
```

### Check Service Status
```powershell
docker-compose ps
docker stats
```

---

## Part 5: Troubleshooting

### Issue 1: Docker Desktop Won't Start

**Solution**:
```powershell
# Check WSL status
wsl --status

# Update WSL
wsl --update

# Set WSL 2 as default
wsl --set-default-version 2

# Restart Docker Desktop
```

---

### Issue 2: "Cannot connect to Docker daemon"

**Solution**:
1. Open Docker Desktop application
2. Wait for Docker Engine to fully start (green icon in system tray)
3. Retry commands

---

### Issue 3: Port Already in Use

**Solution**:
```powershell
# Check what's using port 8000 or 8080
netstat -ano | findstr :8000
netstat -ano | findstr :8080

# Kill process using the port (replace <PID> with actual PID)
taskkill /PID <PID> /F

# Or change port in docker-compose.yml
# Edit ports section: "9000:8000" instead of "8000:8000"
```

---

### Issue 4: Image Load Failed

**Solution**:
```powershell
# Check Docker disk space
docker system df

# Clean up unused images
docker system prune -a

# Retry loading images
docker load -i llama-cpp-server.tar
docker load -i intellistant-v1.0.tar
```

---

### Issue 5: Model File Not Found

**Solution**:
```powershell
# Check model path in docker-compose.yml
# Ensure it matches your model location

# For Qwen model:
# volumes:
#   - ./models/qwen2.5-coder-3b:/models

# For Mistral model:
# volumes:
#   - ./models/mistralai:/models
```

---

### Issue 6: LLM Server Not Responding

**Solution**:
```powershell
# Check llama-server logs
docker-compose logs llama-server

# Restart llama-server
docker-compose restart llama-server

# Wait 30 seconds for model to load
timeout /t 30

# Test again
curl http://localhost:8080/health
```

---

### Issue 7: Permission Denied Errors

**Solution**:
```powershell
# Run PowerShell as Administrator
# Right-click PowerShell -> "Run as Administrator"

# Or adjust Docker Desktop settings:
# Settings -> Resources -> File Sharing
# Add your intellistant directory
```

---

## Part 6: Performance Tuning (Optional)

### Allocate More Resources to Docker

1. Open Docker Desktop
2. Go to Settings → Resources
3. Adjust:
   - **CPUs**: 4-8 cores (for faster inference)
   - **Memory**: 8-16 GB (models need 4-8 GB)
   - **Swap**: 2-4 GB
   - **Disk**: 50+ GB

### GPU Acceleration (if you have NVIDIA GPU)

1. Install NVIDIA Driver on Windows
2. Install NVIDIA Container Toolkit
3. Update docker-compose.yml:
   ```yaml
   llama-server:
     deploy:
       resources:
         reservations:
           devices:
             - driver: nvidia
               count: 1
               capabilities: [gpu]
   ```

---

## Part 7: Testing the Framework

### Test REST API

```powershell
# Health check
curl http://localhost:8000/health

# List agents
curl http://localhost:8000/api/agents

# Chat with CodeAssistant
curl -X POST http://localhost:8000/api/chat `
  -H "Content-Type: application/json" `
  -d '{
    \"user_id\": \"developer_1\",
    \"message\": \"Write a Python function to calculate factorial\"
  }'

# Multi-agent collaboration
curl -X POST http://localhost:8000/api/collaborate `
  -H "Content-Type: application/json" `
  -d '{
    \"user_id\": \"developer_1\",
    \"task\": \"Review and test a login function\",
    \"agents\": [\"CodeAssistant\", \"TestingAgent\", \"SecurityAgent\"]
  }'

# Get metrics
curl http://localhost:8000/api/metrics
```

---

## Part 8: Updating the Framework

### Pull Latest Changes (when back online)

```powershell
cd intellistant
git pull origin main

# Rebuild image
docker-compose down
docker build -t intellistant:v1.0 .
docker-compose up -d
```

### Update Model

```powershell
# Download new model (when online)
# Copy to models/ directory
# Update docker-compose.yml to point to new model
docker-compose restart llama-server
```

---

## Summary Checklist

### Files to Download (with Internet):
- [ ] Docker Desktop Installer.exe (~500 MB)
- [ ] llama-cpp-server.tar (~2-3 GB)
- [ ] intellistant-v1.0.tar (~1-2 GB)
- [ ] GGUF model file (~2-3 GB)
- [ ] Intellistant source code (~500 KB)

### Installation Steps (Offline):
- [ ] Install Docker Desktop
- [ ] Enable WSL 2
- [ ] Load Docker images
- [ ] Extract Intellistant source
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
| Docker Desktop Installer | ~500 MB |
| llama-cpp-server.tar | ~2-3 GB |
| intellistant-v1.0.tar | ~1-2 GB |
| GGUF Model | ~2-3 GB |
| Intellistant Source | ~500 KB |
| **Total** | **~6-9 GB** |

---

## Support

For issues, see:
- [USER_MANUAL.md](docs/USER_MANUAL.md) - Complete user guide
- [DOCKER.md](DOCKER.md) - Docker deployment guide
- [GitHub Issues](https://github.com/pooriayousefi/intellistant/issues)

---

**Last Updated**: December 19, 2025  
**Version**: v1.0.0
