#ifndef POORIAYOUSEFI_MCP_TOOLS_HPP
#define POORIAYOUSEFI_MCP_TOOLS_HPP

#include <filesystem>
#include <fstream>
#include <sstream>
#include <vector>
#include <regex>
#include <sys/stat.h>
#include <unistd.h>
#include "mcp_server.hpp"

namespace pooriayousefi
{

    // ============================================================================
    // Tool Registration Helper
    // ============================================================================

    class ToolRegistry
    {
    public:
        static void register_all_tools(McpServer& server)
        {
            register_filesystem_tools(server);
            register_git_tools(server);
            register_system_tools(server);
        }

        static void register_filesystem_tools(McpServer& server);
        static void register_git_tools(McpServer& server);
        static void register_system_tools(McpServer& server);
    };

    // ============================================================================
    // File System Tools
    // ============================================================================

    namespace tools
    {

        // Read file contents
        inline std::expected<McpToolResult, JsonRpcError> read_file(const nlohmann::json& args)
        {
            try
            {
                std::string path = args.at("path").get<std::string>();
                
                std::ifstream file(path);
                if (!file.is_open())
                {
                    return std::unexpected(JsonRpcError(
                        JsonRpcErrorCode::tool_execution_failed,
                        "Failed to open file: " + path
                    ));
                }
                
                std::stringstream buffer;
                buffer << file.rdbuf();
                
                return McpToolResult::text_result(buffer.str());
            }
            catch (const std::exception& e)
            {
                return std::unexpected(JsonRpcError(
                    JsonRpcErrorCode::tool_execution_failed,
                    std::string("read_file error: ") + e.what()
                ));
            }
        }

        // Write file contents
        inline std::expected<McpToolResult, JsonRpcError> write_file(const nlohmann::json& args)
        {
            try
            {
                std::string path = args.at("path").get<std::string>();
                std::string content = args.at("content").get<std::string>();
                
                // Create parent directories if they don't exist
                std::filesystem::path file_path(path);
                if (file_path.has_parent_path())
                {
                    std::filesystem::create_directories(file_path.parent_path());
                }
                
                std::ofstream file(path);
                if (!file.is_open())
                {
                    return std::unexpected(JsonRpcError(
                        JsonRpcErrorCode::tool_execution_failed,
                        "Failed to open file for writing: " + path
                    ));
                }
                
                file << content;
                file.close();
                
                return McpToolResult::text_result("File written successfully: " + path);
            }
            catch (const std::exception& e)
            {
                return std::unexpected(JsonRpcError(
                    JsonRpcErrorCode::tool_execution_failed,
                    std::string("write_file error: ") + e.what()
                ));
            }
        }

        // List directory contents
        inline std::expected<McpToolResult, JsonRpcError> list_directory(const nlohmann::json& args)
        {
            try
            {
                std::string path = args.at("path").get<std::string>();
                
                if (!std::filesystem::exists(path))
                {
                    return std::unexpected(JsonRpcError(
                        JsonRpcErrorCode::tool_execution_failed,
                        "Directory does not exist: " + path
                    ));
                }
                
                if (!std::filesystem::is_directory(path))
                {
                    return std::unexpected(JsonRpcError(
                        JsonRpcErrorCode::tool_execution_failed,
                        "Path is not a directory: " + path
                    ));
                }
                
                nlohmann::json result = nlohmann::json::array();
                
                for (const auto& entry : std::filesystem::directory_iterator(path))
                {
                    nlohmann::json item;
                    item["name"] = entry.path().filename().string();
                    item["path"] = entry.path().string();
                    item["is_directory"] = entry.is_directory();
                    item["is_file"] = entry.is_regular_file();
                    
                    if (entry.is_regular_file())
                    {
                        item["size"] = std::filesystem::file_size(entry.path());
                    }
                    
                    result.push_back(item);
                }
                
                return McpToolResult::json_result(result);
            }
            catch (const std::exception& e)
            {
                return std::unexpected(JsonRpcError(
                    JsonRpcErrorCode::tool_execution_failed,
                    std::string("list_directory error: ") + e.what()
                ));
            }
        }

        // Search for files matching a pattern
        inline std::expected<McpToolResult, JsonRpcError> search_files(const nlohmann::json& args)
        {
            try
            {
                std::string path = args.at("path").get<std::string>();
                std::string pattern = args.at("pattern").get<std::string>();
                bool recursive = args.value("recursive", true);
                
                if (!std::filesystem::exists(path))
                {
                    return std::unexpected(JsonRpcError(
                        JsonRpcErrorCode::tool_execution_failed,
                        "Path does not exist: " + path
                    ));
                }
                
                std::regex regex_pattern(pattern, std::regex::icase);
                nlohmann::json result = nlohmann::json::array();
                
                auto search_fn = [&](const std::filesystem::path& search_path)
                {
                    if (recursive)
                    {
                        for (const auto& entry : std::filesystem::recursive_directory_iterator(search_path))
                        {
                            if (entry.is_regular_file())
                            {
                                std::string filename = entry.path().filename().string();
                                if (std::regex_search(filename, regex_pattern))
                                {
                                    nlohmann::json item;
                                    item["name"] = filename;
                                    item["path"] = entry.path().string();
                                    item["size"] = std::filesystem::file_size(entry.path());
                                    result.push_back(item);
                                }
                            }
                        }
                    }
                    else
                    {
                        for (const auto& entry : std::filesystem::directory_iterator(search_path))
                        {
                            if (entry.is_regular_file())
                            {
                                std::string filename = entry.path().filename().string();
                                if (std::regex_search(filename, regex_pattern))
                                {
                                    nlohmann::json item;
                                    item["name"] = filename;
                                    item["path"] = entry.path().string();
                                    item["size"] = std::filesystem::file_size(entry.path());
                                    result.push_back(item);
                                }
                            }
                        }
                    }
                };
                
                search_fn(path);
                
                return McpToolResult::json_result(result);
            }
            catch (const std::exception& e)
            {
                return std::unexpected(JsonRpcError(
                    JsonRpcErrorCode::tool_execution_failed,
                    std::string("search_files error: ") + e.what()
                ));
            }
        }

        // Get file information
        inline std::expected<McpToolResult, JsonRpcError> file_info(const nlohmann::json& args)
        {
            try
            {
                std::string path = args.at("path").get<std::string>();
                
                if (!std::filesystem::exists(path))
                {
                    return std::unexpected(JsonRpcError(
                        JsonRpcErrorCode::tool_execution_failed,
                        "File does not exist: " + path
                    ));
                }
                
                nlohmann::json result;
                result["path"] = path;
                result["exists"] = true;
                result["is_directory"] = std::filesystem::is_directory(path);
                result["is_file"] = std::filesystem::is_regular_file(path);
                result["is_symlink"] = std::filesystem::is_symlink(path);
                
                if (std::filesystem::is_regular_file(path))
                {
                    result["size"] = std::filesystem::file_size(path);
                }
                
                // Get modification time
                auto ftime = std::filesystem::last_write_time(path);
                auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                    ftime - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now()
                );
                auto tt = std::chrono::system_clock::to_time_t(sctp);
                result["modified_time"] = std::ctime(&tt);
                
                // Get permissions (Unix-style)
                struct stat st;
                if (stat(path.c_str(), &st) == 0)
                {
                    result["permissions"] = st.st_mode & 0777;
                    result["owner_uid"] = st.st_uid;
                    result["group_gid"] = st.st_gid;
                }
                
                return McpToolResult::json_result(result);
            }
            catch (const std::exception& e)
            {
                return std::unexpected(JsonRpcError(
                    JsonRpcErrorCode::tool_execution_failed,
                    std::string("file_info error: ") + e.what()
                ));
            }
        }

        // Create directory
        inline std::expected<McpToolResult, JsonRpcError> create_directory(const nlohmann::json& args)
        {
            try
            {
                std::string path = args.at("path").get<std::string>();
                bool parents = args.value("parents", true);
                
                if (parents)
                {
                    std::filesystem::create_directories(path);
                }
                else
                {
                    std::filesystem::create_directory(path);
                }
                
                return McpToolResult::text_result("Directory created: " + path);
            }
            catch (const std::exception& e)
            {
                return std::unexpected(JsonRpcError(
                    JsonRpcErrorCode::tool_execution_failed,
                    std::string("create_directory error: ") + e.what()
                ));
            }
        }

        // Delete file or directory
        inline std::expected<McpToolResult, JsonRpcError> delete_path(const nlohmann::json& args)
        {
            try
            {
                std::string path = args.at("path").get<std::string>();
                bool recursive = args.value("recursive", false);
                
                if (!std::filesystem::exists(path))
                {
                    return std::unexpected(JsonRpcError(
                        JsonRpcErrorCode::tool_execution_failed,
                        "Path does not exist: " + path
                    ));
                }
                
                if (recursive)
                {
                    std::filesystem::remove_all(path);
                }
                else
                {
                    std::filesystem::remove(path);
                }
                
                return McpToolResult::text_result("Deleted: " + path);
            }
            catch (const std::exception& e)
            {
                return std::unexpected(JsonRpcError(
                    JsonRpcErrorCode::tool_execution_failed,
                    std::string("delete_path error: ") + e.what()
                ));
            }
        }

    }  // namespace tools

    // ============================================================================
    // Git Tools
    // ============================================================================

    namespace tools
    {

        // Execute git command and return output
        inline std::expected<std::string, std::string> exec_git_command(
            const std::string& repo_path,
            const std::string& command
        )
        {
            std::string full_command = "cd " + repo_path + " && git " + command + " 2>&1";
            
            FILE* pipe = popen(full_command.c_str(), "r");
            if (!pipe)
            {
                return std::unexpected("Failed to execute git command");
            }
            
            std::string result;
            char buffer[256];
            while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
            {
                result += buffer;
            }
            
            int status = pclose(pipe);
            if (status != 0)
            {
                return std::unexpected("Git command failed: " + result);
            }
            
            return result;
        }

        // Get git repository status
        inline std::expected<McpToolResult, JsonRpcError> git_status(const nlohmann::json& args)
        {
            try
            {
                std::string repo_path = args.at("repo_path").get<std::string>();
                
                auto result = exec_git_command(repo_path, "status --porcelain");
                
                if (!result)
                {
                    return std::unexpected(JsonRpcError(
                        JsonRpcErrorCode::tool_execution_failed,
                        "git_status error: " + result.error()
                    ));
                }
                
                return McpToolResult::text_result(*result);
            }
            catch (const std::exception& e)
            {
                return std::unexpected(JsonRpcError(
                    JsonRpcErrorCode::tool_execution_failed,
                    std::string("git_status error: ") + e.what()
                ));
            }
        }

        // Get git log
        inline std::expected<McpToolResult, JsonRpcError> git_log(const nlohmann::json& args)
        {
            try
            {
                std::string repo_path = args.at("repo_path").get<std::string>();
                int limit = args.value("limit", 10);
                
                std::string command = "log -n " + std::to_string(limit) + 
                                    " --pretty=format:'%h|%an|%ad|%s' --date=short";
                
                auto result = exec_git_command(repo_path, command);
                
                if (!result)
                {
                    return std::unexpected(JsonRpcError(
                        JsonRpcErrorCode::tool_execution_failed,
                        "git_log error: " + result.error()
                    ));
                }
                
                return McpToolResult::text_result(*result);
            }
            catch (const std::exception& e)
            {
                return std::unexpected(JsonRpcError(
                    JsonRpcErrorCode::tool_execution_failed,
                    std::string("git_log error: ") + e.what()
                ));
            }
        }

        // Get git diff
        inline std::expected<McpToolResult, JsonRpcError> git_diff(const nlohmann::json& args)
        {
            try
            {
                std::string repo_path = args.at("repo_path").get<std::string>();
                std::string file = args.value("file", "");
                
                std::string command = "diff";
                if (!file.empty())
                {
                    command += " " + file;
                }
                
                auto result = exec_git_command(repo_path, command);
                
                if (!result)
                {
                    return std::unexpected(JsonRpcError(
                        JsonRpcErrorCode::tool_execution_failed,
                        "git_diff error: " + result.error()
                    ));
                }
                
                return McpToolResult::text_result(*result);
            }
            catch (const std::exception& e)
            {
                return std::unexpected(JsonRpcError(
                    JsonRpcErrorCode::tool_execution_failed,
                    std::string("git_diff error: ") + e.what()
                ));
            }
        }

        // List git branches
        inline std::expected<McpToolResult, JsonRpcError> git_branch_list(const nlohmann::json& args)
        {
            try
            {
                std::string repo_path = args.at("repo_path").get<std::string>();
                
                auto result = exec_git_command(repo_path, "branch -a");
                
                if (!result)
                {
                    return std::unexpected(JsonRpcError(
                        JsonRpcErrorCode::tool_execution_failed,
                        "git_branch_list error: " + result.error()
                    ));
                }
                
                return McpToolResult::text_result(*result);
            }
            catch (const std::exception& e)
            {
                return std::unexpected(JsonRpcError(
                    JsonRpcErrorCode::tool_execution_failed,
                    std::string("git_branch_list error: ") + e.what()
                ));
            }
        }

        // Execute shell command
        inline std::expected<McpToolResult, JsonRpcError> execute_command(const nlohmann::json& args)
        {
            try
            {
                std::string command = args.at("command").get<std::string>();
                std::string working_dir = args.value("working_dir", ".");
                
                std::string full_command = "cd " + working_dir + " && " + command + " 2>&1";
                
                FILE* pipe = popen(full_command.c_str(), "r");
                if (!pipe)
                {
                    return std::unexpected(JsonRpcError(
                        JsonRpcErrorCode::tool_execution_failed,
                        "Failed to execute command"
                    ));
                }
                
                std::string result;
                char buffer[256];
                while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
                {
                    result += buffer;
                }
                
                int status = pclose(pipe);
                
                nlohmann::json result_json;
                result_json["output"] = result;
                result_json["exit_code"] = WEXITSTATUS(status);
                
                return McpToolResult::json_result(result_json);
            }
            catch (const std::exception& e)
            {
                return std::unexpected(JsonRpcError(
                    JsonRpcErrorCode::tool_execution_failed,
                    std::string("execute_command error: ") + e.what()
                ));
            }
        }

    }  // namespace tools

    // ============================================================================
    // Tool Registration Implementation
    // ============================================================================

    inline void ToolRegistry::register_filesystem_tools(McpServer& server)
    {
        // read_file
        {
            std::vector<McpToolParameter> params;
            params.push_back({"path", "string", "Path to the file to read", true});
            
            server.register_tool(
                "read_file",
                "Read the contents of a file",
                params,
                tools::read_file
            );
        }

        // write_file
        {
            std::vector<McpToolParameter> params;
            params.push_back({"path", "string", "Path to the file to write", true});
            params.push_back({"content", "string", "Content to write to the file", true});
            
            server.register_tool(
                "write_file",
                "Write content to a file",
                params,
                tools::write_file
            );
        }

        // list_directory
        {
            std::vector<McpToolParameter> params;
            params.push_back({"path", "string", "Path to the directory to list", true});
            
            server.register_tool(
                "list_directory",
                "List contents of a directory",
                params,
                tools::list_directory
            );
        }

        // search_files
        {
            std::vector<McpToolParameter> params;
            params.push_back({"path", "string", "Path to search in", true});
            params.push_back({"pattern", "string", "Regex pattern to match filenames", true});
            params.push_back({"recursive", "boolean", "Search recursively", false, nlohmann::json(true)});
            
            server.register_tool(
                "search_files",
                "Search for files matching a pattern",
                params,
                tools::search_files
            );
        }

        // file_info
        {
            std::vector<McpToolParameter> params;
            params.push_back({"path", "string", "Path to get information about", true});
            
            server.register_tool(
                "file_info",
                "Get detailed information about a file or directory",
                params,
                tools::file_info
            );
        }

        // create_directory
        {
            std::vector<McpToolParameter> params;
            params.push_back({"path", "string", "Path of the directory to create", true});
            params.push_back({"parents", "boolean", "Create parent directories if needed", false, nlohmann::json(true)});
            
            server.register_tool(
                "create_directory",
                "Create a new directory",
                params,
                tools::create_directory
            );
        }

        // delete_path
        {
            std::vector<McpToolParameter> params;
            params.push_back({"path", "string", "Path to delete", true});
            params.push_back({"recursive", "boolean", "Delete recursively (for directories)", false, nlohmann::json(false)});
            
            server.register_tool(
                "delete_path",
                "Delete a file or directory",
                params,
                tools::delete_path
            );
        }
    }

    inline void ToolRegistry::register_git_tools(McpServer& server)
    {
        // git_status
        {
            std::vector<McpToolParameter> params;
            params.push_back({"repo_path", "string", "Path to the git repository", true});
            
            server.register_tool(
                "git_status",
                "Get the status of a git repository",
                params,
                tools::git_status
            );
        }

        // git_log
        {
            std::vector<McpToolParameter> params;
            params.push_back({"repo_path", "string", "Path to the git repository", true});
            params.push_back({"limit", "integer", "Maximum number of commits to show", false, nlohmann::json(10)});
            
            server.register_tool(
                "git_log",
                "Get the commit history of a git repository",
                params,
                tools::git_log
            );
        }

        // git_diff
        {
            std::vector<McpToolParameter> params;
            params.push_back({"repo_path", "string", "Path to the git repository", true});
            params.push_back({"file", "string", "Specific file to show diff for (optional)", false});
            
            server.register_tool(
                "git_diff",
                "Get the diff of changes in a git repository",
                params,
                tools::git_diff
            );
        }

        // git_branch_list
        {
            std::vector<McpToolParameter> params;
            params.push_back({"repo_path", "string", "Path to the git repository", true});
            
            server.register_tool(
                "git_branch_list",
                "List all branches in a git repository",
                params,
                tools::git_branch_list
            );
        }
    }

    inline void ToolRegistry::register_system_tools(McpServer& server)
    {
        // execute_command
        {
            std::vector<McpToolParameter> params;
            params.push_back({"command", "string", "Shell command to execute", true});
            params.push_back({"working_dir", "string", "Working directory for command execution", false, nlohmann::json(".")});
            
            server.register_tool(
                "execute_command",
                "Execute a shell command and return the output",
                params,
                tools::execute_command
            );
        }
    }

}  // namespace pooriayousefi

#endif  // POORIAYOUSEFI_MCP_TOOLS_HPP
