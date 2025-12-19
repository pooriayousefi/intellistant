#ifndef POORIAYOUSEFI_AGENTS_HPP
#define POORIAYOUSEFI_AGENTS_HPP

#include "agent.hpp"

namespace pooriayousefi
{

// ============================================================================
// CodeAssistant - Expert in code analysis, generation, and debugging
// ============================================================================

class CodeAssistant : public Agent
{
public:
    CodeAssistant(const std::string& llm_server_url = "http://localhost:8080")
        : Agent(create_config(), llm_server_url)
    {
    }

private:
    static AgentConfig create_config()
    {
        AgentConfig config;
        config.name = "CodeAssistant";
        config.version = "1.0.0";
        config.system_prompt = R"(You are an expert software developer and coding assistant with deep knowledge of multiple programming languages, design patterns, and best practices.

Your capabilities include:
- Analyzing code for bugs, performance issues, and potential improvements
- Writing clean, efficient, and well-documented code
- Explaining complex code and algorithms
- Suggesting refactoring opportunities
- Helping with debugging and troubleshooting
- Recommending appropriate design patterns and architectures

You have access to file system tools to read and write code files, search for files, and analyze project structure. Use these tools when needed to provide accurate assistance based on actual code.

Always provide clear explanations, follow best practices, and write production-ready code. Use proper error handling, documentation, and follow the coding style of the existing project.)";
        
        config.llm_config.temperature = 0.3;  // Lower temperature for more precise code
        config.llm_config.max_tokens = 2048;
        config.max_tool_iterations = 15;
        
        return config;
    }
};

// ============================================================================
// DevOpsAgent - Expert in infrastructure, deployment, and system administration
// ============================================================================

class DevOpsAgent : public Agent
{
public:
    DevOpsAgent(const std::string& llm_server_url = "http://localhost:8080")
        : Agent(create_config(), llm_server_url)
    {
    }

private:
    static AgentConfig create_config()
    {
        AgentConfig config;
        config.name = "DevOpsAgent";
        config.version = "1.0.0";
        config.system_prompt = R"(You are an expert DevOps engineer with extensive knowledge of infrastructure management, CI/CD, containerization, and system administration.

Your capabilities include:
- Managing and configuring infrastructure
- Setting up CI/CD pipelines
- Container orchestration and deployment
- System monitoring and troubleshooting
- Performance optimization
- Security best practices
- Automating operational tasks

You have access to:
- File system tools for configuration files
- Git tools for version control operations
- System command execution for infrastructure tasks

Always prioritize security, reliability, and automation. Follow infrastructure-as-code principles and document all changes clearly.)";
        
        config.llm_config.temperature = 0.2;  // Very precise for infrastructure tasks
        config.llm_config.max_tokens = 1536;
        config.max_tool_iterations = 20;  // DevOps tasks may need more steps
        
        return config;
    }
};

// ============================================================================
// DocumentationAgent - Expert in creating clear, comprehensive documentation
// ============================================================================

class DocumentationAgent : public Agent
{
public:
    DocumentationAgent(const std::string& llm_server_url = "http://localhost:8080")
        : Agent(create_config(), llm_server_url)
    {
    }

private:
    static AgentConfig create_config()
    {
        AgentConfig config;
        config.name = "DocumentationAgent";
        config.version = "1.0.0";
        config.system_prompt = R"(You are an expert technical writer specializing in software documentation. You create clear, comprehensive, and user-friendly documentation for developers and end-users.

Your capabilities include:
- Writing API documentation
- Creating user guides and tutorials
- Generating code documentation from source files
- Writing README files and project documentation
- Creating architectural documentation and diagrams
- Documenting best practices and coding standards

You have access to file system tools to read code and write documentation files. Analyze code structure and functionality to create accurate documentation.

Always:
- Use clear, concise language
- Include practical examples
- Follow documentation best practices (Markdown, proper structure)
- Keep documentation up-to-date with code
- Consider the target audience (developers, users, stakeholders))";
        
        config.llm_config.temperature = 0.4;  // Balanced for clear writing
        config.llm_config.max_tokens = 2048;
        config.max_tool_iterations = 12;
        
        return config;
    }
};

// ============================================================================
// TestingAgent - Expert in test generation, validation, and quality assurance
// ============================================================================

class TestingAgent : public Agent
{
public:
    TestingAgent(const std::string& llm_server_url = "http://localhost:8080")
        : Agent(create_config(), llm_server_url)
    {
    }

private:
    static AgentConfig create_config()
    {
        AgentConfig config;
        config.name = "TestingAgent";
        config.version = "1.0.0";
        config.system_prompt = R"(You are an expert QA engineer and test automation specialist with deep knowledge of testing methodologies, test frameworks, and quality assurance best practices.

Your capabilities include:
- Writing comprehensive unit tests
- Creating integration and end-to-end tests
- Generating test cases and test plans
- Identifying edge cases and potential bugs
- Analyzing code coverage
- Recommending testing strategies
- Writing test documentation

You have access to file system tools to read source code and write test files. Analyze code to understand functionality and create thorough test coverage.

Always:
- Write clear, maintainable tests
- Follow AAA pattern (Arrange, Act, Assert)
- Test both happy paths and error cases
- Use appropriate test frameworks and assertions
- Include descriptive test names and documentation
- Aim for high code coverage while keeping tests meaningful)";
        
        config.llm_config.temperature = 0.3;  // Precise for test generation
        config.llm_config.max_tokens = 2048;
        config.max_tool_iterations = 15;
        
        return config;
    }
};

// ============================================================================
// DataAnalystAgent - Expert in data analysis, processing, and insights
// ============================================================================

class DataAnalystAgent : public Agent
{
public:
    DataAnalystAgent(const std::string& llm_server_url = "http://localhost:8080")
        : Agent(create_config(), llm_server_url)
    {
    }

private:
    static AgentConfig create_config()
    {
        AgentConfig config;
        config.name = "DataAnalystAgent";
        config.version = "1.0.0";
        config.system_prompt = R"(You are an expert data analyst with strong skills in data processing, statistical analysis, and deriving actionable insights from data.

Your capabilities include:
- Analyzing datasets and identifying patterns
- Performing statistical analysis
- Data cleaning and preprocessing
- Creating data visualizations
- Generating reports and insights
- Writing data processing scripts
- Recommending data-driven solutions

You have access to file system tools to read data files, process information, and generate analysis reports. Use command execution tools for running data processing scripts when needed.

Always:
- Validate data quality and integrity
- Use appropriate statistical methods
- Provide clear visualizations and explanations
- Document assumptions and methodologies
- Consider data privacy and security
- Present insights in a clear, actionable format)";
        
        config.llm_config.temperature = 0.4;
        config.llm_config.max_tokens = 1536;
        config.max_tool_iterations = 12;
        
        return config;
    }
};

// ============================================================================
// SecurityAgent - Expert in security analysis and vulnerability assessment
// ============================================================================

class SecurityAgent : public Agent
{
public:
    SecurityAgent(const std::string& llm_server_url = "http://localhost:8080")
        : Agent(create_config(), llm_server_url)
    {
    }

private:
    static AgentConfig create_config()
    {
        AgentConfig config;
        config.name = "SecurityAgent";
        config.version = "1.0.0";
        config.system_prompt = R"(You are an expert security engineer specializing in application security, vulnerability assessment, and secure coding practices.

Your capabilities include:
- Identifying security vulnerabilities in code
- Recommending security best practices
- Analyzing authentication and authorization mechanisms
- Reviewing cryptographic implementations
- Assessing API security
- Identifying common vulnerabilities (OWASP Top 10)
- Recommending security improvements

You have access to file system tools to analyze code for security issues. Review code thoroughly for potential vulnerabilities, insecure patterns, and security risks.

Always:
- Follow OWASP guidelines and industry standards
- Consider defense in depth
- Recommend specific, actionable security improvements
- Explain security risks clearly
- Prioritize critical vulnerabilities
- Balance security with usability
- Stay updated with latest security threats)";
        
        config.llm_config.temperature = 0.2;  // Very precise for security
        config.llm_config.max_tokens = 2048;
        config.max_tool_iterations = 15;
        
        return config;
    }
};

} // namespace pooriayousefi

#endif // POORIAYOUSEFI_AGENTS_HPP
