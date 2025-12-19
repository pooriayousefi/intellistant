# Intellistant Framework - Production Docker Image
# Multi-stage build for optimized image size

# Stage 1: Build stage
FROM ubuntu:22.04 AS builder

# Avoid interactive prompts
ENV DEBIAN_FRONTEND=noninteractive

# Install build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    wget \
    libcurl4-openssl-dev \
    && rm -rf /var/lib/apt/lists/*

# Install GCC 14 (for C++23 support)
RUN apt-get update && apt-get install -y software-properties-common \
    && add-apt-repository ppa:ubuntu-toolchain-r/test \
    && apt-get update \
    && apt-get install -y gcc-14 g++-14 \
    && update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-14 100 \
    && update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-14 100 \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /app

# Copy source code
COPY include/ ./include/
COPY src/ ./src/
COPY tests/ ./tests/
COPY jsonrpc2/ ./jsonrpc2/
COPY CMakeLists.txt ./

# Build the project
RUN mkdir -p build && cd build \
    && cmake .. \
    && make -j$(nproc) \
    && echo "Build completed successfully"

# Stage 2: Runtime stage (smaller image)
FROM ubuntu:22.04

# Avoid interactive prompts
ENV DEBIAN_FRONTEND=noninteractive

# Install runtime dependencies only
RUN apt-get update && apt-get install -y \
    libcurl4-openssl-dev \
    libstdc++6 \
    curl \
    wget \
    && rm -rf /var/lib/apt/lists/*

# Install GCC 14 runtime libraries
RUN apt-get update && apt-get install -y software-properties-common \
    && add-apt-repository ppa:ubuntu-toolchain-r/test \
    && apt-get update \
    && apt-get install -y libstdc++6 \
    && rm -rf /var/lib/apt/lists/*

# Create app directory
WORKDIR /app

# Copy built executables from builder stage
COPY --from=builder /app/build/intellistant_server ./intellistant_server
COPY --from=builder /app/build/intellistant_cli ./intellistant_cli
COPY --from=builder /app/build/llm_client_tests ./llm_client_tests
COPY --from=builder /app/build/mcp_tools_tests ./mcp_tools_tests
COPY --from=builder /app/build/agent_tests ./agent_tests
COPY --from=builder /app/build/coordinator_tests ./coordinator_tests

# Copy include files (needed for header-only architecture)
COPY --from=builder /app/include/ ./include/
COPY --from=builder /app/jsonrpc2/include/ ./jsonrpc2/include/

# Copy documentation
COPY docs/ ./docs/
COPY README.md CHANGELOG.md LICENSE ./

# Create directories for runtime
RUN mkdir -p /app/logs /app/data

# Environment variables
ENV INTELLISTANT_HOME=/app
ENV LLAMA_HOST=llama-server
ENV LLAMA_PORT=8080
ENV API_PORT=8000

# Health check
HEALTHCHECK --interval=30s --timeout=10s --start-period=5s --retries=3 \
    CMD curl -f http://localhost:${API_PORT}/health || exit 1

# Expose API server port
EXPOSE 8000

# Default command: run API server
CMD ["./intellistant_server", "--host", "0.0.0.0", "--port", "8000"]
