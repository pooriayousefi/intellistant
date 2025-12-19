cmake_minimum_required(VERSION 3.20)

project(intellistant VERSION 1.0.0 LANGUAGES CXX)

# Set C++23 standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/jsonrpc2/include
)

# Find threads library
find_package(Threads REQUIRED)

# Enable testing
enable_testing()

# LLM Client Tests
add_executable(llm_client_tests
    tests/llm_client_tests.cpp
)

target_link_libraries(llm_client_tests
    PRIVATE
    Threads::Threads
)

# Add test
add_test(NAME llm_client_tests COMMAND llm_client_tests)

# Set output directory
set_target_properties(llm_client_tests
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Compiler-specific options
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(llm_client_tests PRIVATE
        -Wall
        -Wextra
        -O2
    )
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    target_compile_options(llm_client_tests PRIVATE
        /W4
        /O2
        /std:c++latest
    )
endif()

# MCP Tools Tests
add_executable(mcp_tools_tests
    tests/mcp_tools_tests.cpp
)

target_link_libraries(mcp_tools_tests
    PRIVATE
    Threads::Threads
)

# Add test
add_test(NAME mcp_tools_tests COMMAND mcp_tools_tests)

# Set output directory
set_target_properties(mcp_tools_tests
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Compiler-specific options
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(mcp_tools_tests PRIVATE
        -Wall
        -Wextra
        -O2
    )
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    target_compile_options(mcp_tools_tests PRIVATE
        /W4
        /O2
        /std:c++latest
    )
endif()

# Print build information
message(STATUS "C++ Standard: C++${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")

# Agent Tests
add_executable(agent_tests
    tests/agent_tests.cpp
)

target_link_libraries(agent_tests
    PRIVATE
    Threads::Threads
)

# Add test
add_test(NAME agent_tests COMMAND agent_tests)

# Set output directory
set_target_properties(agent_tests
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Compiler-specific options
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(agent_tests PRIVATE
        -Wall
        -Wextra
        -O2
    )
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    target_compile_options(agent_tests PRIVATE
        /W4
        /O2
        /std:c++latest
    )
endif()

# Agent Demo
add_executable(agent_demo
    examples/agent_demo.cpp
)

target_link_libraries(agent_demo
    PRIVATE
    Threads::Threads
)

# Set output directory
set_target_properties(agent_demo
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Compiler-specific options
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(agent_demo PRIVATE
        -Wall
        -Wextra
        -O2
    )
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    target_compile_options(agent_demo PRIVATE
        /W4
        /O2
        /std:c++latest
    )
endif()

# MCP Demo
add_executable(mcp_demo
    examples/mcp_demo.cpp
)

target_link_libraries(mcp_demo
    PRIVATE
    Threads::Threads
)

# Set output directory
set_target_properties(mcp_demo
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Compiler-specific options
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(mcp_demo PRIVATE
        -Wall
        -Wextra
        -O2
    )
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    target_compile_options(mcp_demo PRIVATE
        /W4
        /O2
        /std:c++latest
    )
endif()

# Coordinator Tests
add_executable(coordinator_tests
    tests/coordinator_tests.cpp
)

target_link_libraries(coordinator_tests
    PRIVATE
    Threads::Threads
)

# Add test
add_test(NAME coordinator_tests COMMAND coordinator_tests)

# Set output directory
set_target_properties(coordinator_tests
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Compiler-specific options
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(coordinator_tests PRIVATE
        -Wall
        -Wextra
        -O2
    )
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    target_compile_options(coordinator_tests PRIVATE
        /W4
        /O2
        /std:c++latest
    )
endif()

# Coordinator Demo
add_executable(coordinator_demo
    examples/coordinator_demo.cpp
)

target_link_libraries(coordinator_demo
    PRIVATE
    Threads::Threads
)

# Set output directory
set_target_properties(coordinator_demo
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Compiler-specific options
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(coordinator_demo PRIVATE
        -Wall
        -Wextra
        -O2
    )
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    target_compile_options(coordinator_demo PRIVATE
        /W4
        /O2
        /std:c++latest
    )
endif()

# CLI Tool
add_executable(intellistant_cli
    src/intellistant_cli.cpp
)

target_link_libraries(intellistant_cli
    PRIVATE
    Threads::Threads
)

# Set output directory
set_target_properties(intellistant_cli
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Compiler-specific options
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(intellistant_cli PRIVATE
        -Wall
        -Wextra
        -O2
    )
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    target_compile_options(intellistant_cli PRIVATE
        /W4
        /O2
        /std:c++latest
    )
endif()

# API Server
add_executable(intellistant_server
    src/intellistant_server.cpp
)

target_link_libraries(intellistant_server
    PRIVATE
    Threads::Threads
)

# Set output directory
set_target_properties(intellistant_server
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Compiler-specific options
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(intellistant_server PRIVATE
        -Wall
        -Wextra
        -O2
    )
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    target_compile_options(intellistant_server PRIVATE
        /W4
        /O2
        /std:c++latest
    )
endif()
