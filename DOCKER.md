# 🐳 Docker Quick Start Guide

This guide shows you how to run Intellistant using Docker.

## Prerequisites

- Docker 20.10+ installed
- Docker Compose 2.0+ installed
- At least 8GB RAM available
- 10GB free disk space

## Quick Start (Recommended)

### 1. Clone the Repository

```bash
git clone https://github.com/pooriayousefi/intellistant.git
cd intellistant
```

### 2. Download a Model (Required)

You need a GGUF model for llama.cpp. Download one to the `models/` directory:

```bash
# Example: Download Qwen 2.5 Coder 3B (recommended)
mkdir -p models/qwen2.5-coder-3b
cd models/qwen2.5-coder-3b

# Use huggingface-cli or wget
wget https://huggingface.co/Qwen/Qwen2.5-Coder-3B-Instruct-GGUF/resolve/main/qwen2.5-coder-3b-instruct-q4_k_m.gguf \
  -O instruct-q4_k_m.gguf

cd ../..
```

**Alternative models**:
- Mistral 7B: `mistralai/Mistral-7B-Instruct-v0.2-GGUF`
- Llama 3.1 8B: `meta-llama/Llama-3.1-8B-Instruct-GGUF`
- Any GGUF format model from Hugging Face

### 3. Start the Services

```bash
# Start llama-server + API server
docker-compose up -d

# Check logs
docker-compose logs -f
```

**Wait ~30 seconds** for llama-server to load the model.

### 4. Test the API

```bash
# Health check
curl http://localhost:8000/health

# List agents
curl http://localhost:8000/api/agents

# Chat with agents
curl -X POST http://localhost:8000/api/chat \
  -H "Content-Type: application/json" \
  -d '{
    "user_id": "demo_user",
    "message": "Write a Python function to calculate fibonacci numbers"
  }'
```

### 5. Use the CLI (Optional)

```bash
# Run CLI in interactive mode
docker-compose run --rm intellistant-cli

# Or attach to running container
docker exec -it intellistant-cli ./intellistant_cli
```

---

## Architecture

```
┌─────────────────────────────────────────────┐
│          Docker Compose Services            │
├─────────────────────────────────────────────┤
│                                             │
│  ┌────────────────┐    ┌─────────────────┐ │
│  │  llama-server  │◄───┤ intellistant-api│ │
│  │  (Port 8080)   │    │  (Port 8000)    │ │
│  │                │    │                 │ │
│  │  • Model loaded│    │ • REST API      │ │
│  │  • Inference   │    │ • 8 endpoints   │ │
│  └────────────────┘    └─────────────────┘ │
│                                             │
│  ┌────────────────┐                         │
│  │intellistant-cli│ (optional)              │
│  │  Interactive   │                         │
│  └────────────────┘                         │
│                                             │
└─────────────────────────────────────────────┘
         │                    │
         │                    │
    Port 8080            Port 8000
   (internal)          (exposed)
```

---

## Configuration

### Environment Variables

Edit `docker-compose.yml` to customize:

```yaml
environment:
  - LLAMA_HOST=llama-server    # LLM server hostname
  - LLAMA_PORT=8080            # LLM server port
  - API_PORT=8000              # API server port
```

### Model Selection

To use a different model, edit `docker-compose.yml`:

```yaml
llama-server:
  command: >
    --model /models/YOUR_MODEL/model.gguf
    --host 0.0.0.0
    --port 8080
```

### Resource Limits

Adjust memory limits in `docker-compose.yml`:

```yaml
deploy:
  resources:
    limits:
      memory: 8G    # Increase for larger models
```

---

## Common Commands

### Start Services
```bash
# Start in background
docker-compose up -d

# Start with logs
docker-compose up

# Start specific service
docker-compose up llama-server
```

### Stop Services
```bash
# Stop all services
docker-compose down

# Stop and remove volumes
docker-compose down -v
```

### View Logs
```bash
# All services
docker-compose logs -f

# Specific service
docker-compose logs -f intellistant-api
docker-compose logs -f llama-server
```

### Check Status
```bash
# List running containers
docker-compose ps

# Health checks
curl http://localhost:8000/health
curl http://localhost:8080/health
```

### Run Tests
```bash
# Run all tests inside container
docker-compose exec intellistant-api ./llm_client_tests
docker-compose exec intellistant-api ./mcp_tools_tests
docker-compose exec intellistant-api ./agent_tests
docker-compose exec intellistant-api ./coordinator_tests
```

### Interactive CLI
```bash
# Option 1: Run new container
docker-compose run --rm intellistant-cli

# Option 2: Exec into running API container
docker exec -it intellistant-api ./intellistant_cli
```

---

## Troubleshooting

### Issue: llama-server won't start

**Symptom**: `llama-server` exits immediately

**Solutions**:
1. Check model path: Ensure model file exists in `models/` directory
2. Check memory: llama-server needs 4-8GB RAM
3. Check logs: `docker-compose logs llama-server`

```bash
# Verify model exists
ls -lh models/qwen2.5-coder-3b/instruct-q4_k_m.gguf

# Increase memory limit in docker-compose.yml
deploy:
  resources:
    limits:
      memory: 16G  # For larger models
```

### Issue: API server can't connect to llama-server

**Symptom**: API returns "Connection refused"

**Solutions**:
1. Wait for llama-server health check: `docker-compose ps`
2. Check network: `docker network ls | grep intellistant`
3. Restart services: `docker-compose restart`

```bash
# Check llama-server health
curl http://localhost:8080/health

# Check API can reach llama-server
docker exec intellistant-api curl http://llama-server:8080/health
```

### Issue: Out of memory

**Symptom**: Services killed by OOM

**Solutions**:
1. Use smaller model (e.g., 3B instead of 7B)
2. Increase Docker memory limit
3. Close other applications

```bash
# Check Docker memory usage
docker stats

# Use quantized model (Q4_K_M instead of F16)
# Q4_K_M: ~2GB, Q8_0: ~4GB, F16: ~14GB
```

### Issue: Slow responses

**Symptom**: API takes >10s to respond

**Solutions**:
1. Use GPU: Add `--n-gpu-layers 99` to llama-server command
2. Reduce context size: `--ctx-size 2048` (default 4096)
3. Use smaller model
4. Increase threads: `--threads 8`

```yaml
llama-server:
  command: >
    --model /models/model.gguf
    --n-gpu-layers 99      # Use GPU
    --threads 8            # More CPU threads
    --ctx-size 2048        # Smaller context
```

### Issue: Port already in use

**Symptom**: `bind: address already in use`

**Solutions**:
1. Change port in `docker-compose.yml`
2. Stop conflicting service
3. Use different port mapping

```yaml
ports:
  - "8001:8000"  # Use 8001 instead of 8000
```

---

## Development with Docker

### Build Custom Image

```bash
# Build with custom tag
docker build -t intellistant:dev .

# Build with specific GCC version
docker build --build-arg GCC_VERSION=14 -t intellistant:gcc14 .
```

### Mount Local Code

For development, mount your code:

```yaml
intellistant-api:
  volumes:
    - ./include:/app/include:ro
    - ./src:/app/src:ro
    - ./build:/app/build
```

### Debug Mode

Run container with shell access:

```bash
# Get shell in API container
docker exec -it intellistant-api /bin/bash

# Or start container with bash
docker run -it --rm intellistant:latest /bin/bash
```

---

## Production Deployment

### Docker Swarm

```bash
# Initialize swarm
docker swarm init

# Deploy stack
docker stack deploy -c docker-compose.yml intellistant

# Check services
docker service ls
```

### Kubernetes

Convert docker-compose to Kubernetes:

```bash
# Install kompose
curl -L https://github.com/kubernetes/kompose/releases/download/v1.30.0/kompose-linux-amd64 -o kompose
chmod +x kompose

# Convert
./kompose convert -f docker-compose.yml

# Deploy
kubectl apply -f .
```

### Scale API Servers

```bash
# Scale to 3 replicas
docker-compose up -d --scale intellistant-api=3

# Or in Kubernetes
kubectl scale deployment intellistant-api --replicas=3
```

---

## Performance Tuning

### CPU Optimization

```yaml
llama-server:
  command: >
    --threads $(nproc)  # Use all CPU cores
    --batch-size 512    # Larger batches
```

### Memory Optimization

```yaml
llama-server:
  command: >
    --ctx-size 2048     # Smaller context window
    --n-batch 256       # Smaller batch size
```

### GPU Acceleration

```yaml
llama-server:
  deploy:
    resources:
      reservations:
        devices:
          - driver: nvidia
            count: 1
            capabilities: [gpu]
  command: >
    --n-gpu-layers 99   # Offload all layers to GPU
```

---

## Clean Up

### Remove All Containers and Images

```bash
# Stop and remove containers
docker-compose down

# Remove images
docker rmi intellistant:latest

# Remove volumes (WARNING: deletes data)
docker-compose down -v

# Full cleanup
docker system prune -a
```

---

## Next Steps

- Read [USER_MANUAL.md](docs/USER_MANUAL.md) for complete API documentation
- See [examples/](examples/) for real-world use cases
- Join discussions on [GitHub Issues](https://github.com/pooriayousefi/intellistant/issues)

---

**Questions?** Open an issue on GitHub!
