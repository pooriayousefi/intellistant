# Contributing to Intellistant

Thank you for your interest in contributing to **Intellistant**! This document provides guidelines for contributing to the project.

## 🎯 Ways to Contribute

- **Bug Reports**: Report bugs through GitHub Issues
- **Feature Requests**: Suggest new features or improvements
- **Code Contributions**: Submit pull requests for bug fixes or features
- **Documentation**: Improve or add documentation
- **Testing**: Write tests or improve test coverage
- **Examples**: Contribute usage examples or tutorials

## 🚀 Getting Started

### Prerequisites

- C++23 compiler (GCC 14+, Clang 17+)
- CMake 3.20+
- Git
- llama.cpp server (for testing)

### Setup Development Environment

```bash
# Clone the repository
git clone https://github.com/pooriayousefi/intellistant.git
cd intellistant

# Create build directory
mkdir -p build && cd build

# Build the project
cmake .. && make -j4

# Run all tests
./llm_client_tests
./mcp_tools_tests
./agent_tests
./coordinator_tests
```

## 📋 Pull Request Process

1. **Fork the Repository**
   ```bash
   git clone https://github.com/YOUR_USERNAME/intellistant.git
   cd intellistant
   git remote add upstream https://github.com/pooriayousefi/intellistant.git
   ```

2. **Create a Feature Branch**
   ```bash
   git checkout -b feature/your-feature-name
   # or
   git checkout -b fix/your-bug-fix
   ```

3. **Make Your Changes**
   - Write clean, readable C++23 code
   - Follow the existing code style
   - Add tests for new functionality
   - Update documentation as needed

4. **Test Your Changes**
   ```bash
   cd build
   cmake .. && make -j4
   ./llm_client_tests
   ./mcp_tools_tests
   ./agent_tests
   ./coordinator_tests
   ```

5. **Commit Your Changes**
   ```bash
   git add .
   git commit -m "feat: Add new feature X"
   # or
   git commit -m "fix: Fix bug in Y"
   ```

   **Commit Message Convention**:
   - `feat:` - New feature
   - `fix:` - Bug fix
   - `docs:` - Documentation changes
   - `test:` - Test additions/changes
   - `refactor:` - Code refactoring
   - `perf:` - Performance improvements
   - `chore:` - Build/tooling changes

6. **Push to Your Fork**
   ```bash
   git push origin feature/your-feature-name
   ```

7. **Open a Pull Request**
   - Go to GitHub and open a PR from your fork
   - Provide a clear description of your changes
   - Reference any related issues
   - Ensure all tests pass

## 💻 Code Style Guidelines

### C++ Style

- **Standard**: Use C++23 features where appropriate
- **Naming**:
  - Classes: `PascalCase` (e.g., `LLMClient`, `AgentFactory`)
  - Functions: `snake_case` (e.g., `create_session`, `get_agents`)
  - Variables: `snake_case` (e.g., `session_id`, `agent_name`)
  - Constants: `UPPER_SNAKE_CASE` (e.g., `MAX_RETRIES`)
- **Indentation**: 4 spaces (no tabs)
- **Headers**: Header-only when possible
- **Error Handling**: Use `std::expected` instead of exceptions
- **Comments**: Use `//` for single-line, `/* */` for multi-line

### Example

```cpp
namespace pooriayousefi {

class NewAgent : public Agent
{
public:
    // Constructor with clear parameter names
    NewAgent(const std::string& llm_host, int max_iterations = 5)
        : Agent(llm_host, "NewAgent", get_system_prompt(), get_available_tools(), max_iterations)
    {}

    // Use std::expected for error handling
    std::expected<Response, Error> process_request(const Request& req)
    {
        if (!req.is_valid()) {
            return std::unexpected(Error{"Invalid request"});
        }
        
        // Implementation...
        return Response{};
    }

private:
    static std::string get_system_prompt()
    {
        return "You are a helpful assistant...";
    }
};

} // namespace pooriayousefi
```

## 🧪 Testing Guidelines

### Writing Tests

- Add tests for all new features
- Use descriptive test names
- Test both success and error cases
- Keep tests independent

### Test Structure

```cpp
void test_new_feature()
{
    std::cout << "========================================\n";
    std::cout << "TEST: New Feature\n";
    std::cout << "========================================\n";
    
    // Setup
    auto client = create_test_client();
    
    // Execute
    auto result = client->new_feature();
    
    // Verify
    if (result && result->is_valid()) {
        std::cout << "[✓] New feature works\n";
    } else {
        std::cout << "[✗] New feature failed\n";
        std::exit(1);
    }
}
```

### Running Tests

```bash
# Run all test suites
cd build
./llm_client_tests      # Phase 1 tests
./mcp_tools_tests       # Phase 2 tests
./agent_tests           # Phase 3 tests
./coordinator_tests     # Phase 4 tests

# Run with verbose output
./llm_client_tests --verbose
```

## 📚 Documentation Guidelines

### Code Documentation

- Document all public APIs
- Use clear, concise comments
- Provide usage examples

```cpp
/**
 * @brief Creates a new session for a user
 * 
 * @param user_id Unique identifier for the user
 * @return Session ID string (format: "req_<timestamp>")
 * 
 * @example
 * auto coordinator = Coordinator::create("http://localhost:8080");
 * std::string session = coordinator->create_session("user_123");
 */
std::string create_session(const std::string& user_id);
```

### Markdown Documentation

- Update relevant `.md` files when adding features
- Follow existing structure and style
- Include code examples
- Add to table of contents if creating new sections

## 🐛 Bug Reports

When reporting bugs, please include:

1. **Description**: Clear description of the bug
2. **Steps to Reproduce**: Exact steps to reproduce the issue
3. **Expected Behavior**: What you expected to happen
4. **Actual Behavior**: What actually happened
5. **Environment**:
   - OS and version (e.g., Ubuntu 22.04)
   - Compiler and version (e.g., GCC 14.1)
   - CMake version
   - Relevant configuration

### Bug Report Template

```markdown
## Bug Description
Brief description of the bug

## Steps to Reproduce
1. Build the project with `cmake .. && make`
2. Run `./intellistant_cli`
3. Execute command `/agents`
4. Observe error

## Expected Behavior
The CLI should list all 6 agents

## Actual Behavior
CLI crashes with segmentation fault

## Environment
- OS: Ubuntu 22.04
- Compiler: GCC 14.1
- CMake: 3.28
- llama.cpp: latest main branch

## Additional Context
Any other relevant information
```

## 💡 Feature Requests

When requesting features:

1. **Use Case**: Describe the problem you're trying to solve
2. **Proposed Solution**: Your suggested implementation
3. **Alternatives**: Other solutions you've considered
4. **Additional Context**: Any other relevant information

## 🔍 Code Review Process

All PRs will be reviewed for:

- **Correctness**: Does the code work as intended?
- **Tests**: Are there adequate tests?
- **Style**: Does it follow project style guidelines?
- **Documentation**: Is it properly documented?
- **Performance**: Are there any performance concerns?
- **Compatibility**: Does it maintain backward compatibility?

## 📝 Additional Resources

- **[USER_MANUAL.md](docs/USER_MANUAL.md)** - Complete user guide
- **[STATUS.md](docs/STATUS.md)** - Project status
- **[ROADMAP.md](ROADMAP.md)** - Development roadmap
- **Phase Documentation**:
  - [PHASE2_COMPLETE.md](docs/PHASE2_COMPLETE.md) - MCP Tools
  - [PHASE3_COMPLETE.md](docs/PHASE3_COMPLETE.md) - Agent System
  - [PHASE4_COMPLETE.md](docs/PHASE4_COMPLETE.md) - Coordinator
  - [PHASE5_COMPLETE.md](docs/PHASE5_COMPLETE.md) - REST API & CLI

## 🤝 Community

- **GitHub Issues**: For bugs and feature requests
- **Pull Requests**: For code contributions
- **Discussions**: For questions and general discussion

## 📜 Code of Conduct

### Our Pledge

We are committed to providing a welcoming and inspiring community for all.

### Our Standards

**Positive Behavior**:
- Be respectful and considerate
- Welcome newcomers and help them learn
- Focus on what is best for the community
- Show empathy towards others

**Unacceptable Behavior**:
- Harassment or discriminatory language
- Personal attacks or trolling
- Publishing others' private information
- Other unethical or unprofessional conduct

### Enforcement

Instances of abusive behavior may be reported to the project maintainers.

## 🙏 Thank You!

Thank you for contributing to Intellistant! Every contribution, no matter how small, helps make this project better.

---

**Questions?** Open an issue or reach out to [@pooriayousefi](https://github.com/pooriayousefi)
