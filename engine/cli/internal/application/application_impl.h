#pragma once

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

// Forward declarations to avoid including unnecessary headers
namespace cortex {
namespace commands {
class CommandRegistry;
}
}

class DatabaseService;
class DownloadService;

namespace cortex {

// Implementation class for Application (Pimpl pattern)
class Application::Impl {
public:
    Impl();
    ~Impl();

    // Services
    std::shared_ptr<DatabaseService> db_service_;
    std::shared_ptr<DownloadService> download_service_;
    
    // Command registry
    std::shared_ptr<commands::CommandRegistry> command_registry_;
    
    /**
     * @brief Initialize services
     * 
     * @return bool True if initialization was successful
     */
    bool InitializeServices();
    
    /**
     * @brief Register commands with the registry
     */
    void RegisterCommands();
    
    /**
     * @brief Parse command line arguments
     * 
     * @param argc Argument count
     * @param argv Argument values
     * @param command_name Output parameter for command name
     * @param args Output parameter for command arguments
     * @param options Output parameter for command options
     */
    void ParseArgs(int argc, char* argv[], 
                  std::string& command_name,
                  std::vector<std::string>& args,
                  std::unordered_map<std::string, std::string>& options);
                  
    /**
     * @brief Check if system is supported
     * 
     * @return bool True if system is supported
     */
    bool IsSystemSupported() const;
    
    /**
     * @brief Initialize system components
     * 
     * @return bool True if initialization was successful
     */
    bool InitializeSystem();
    
    /**
     * @brief Cleanup system resources
     */
    void CleanupSystem();
};

} // namespace cortex