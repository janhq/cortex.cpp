// commands/public/application.h
#pragma once

#include <memory>
#include <string>

namespace cortex {

/**
 * @brief Main application class
 * 
 * This class handles command-line arguments, initializes the application, and
 * orchestrates command execution.
 */
class Application {
public:
    /**
     * @brief Construct a new Application
     */
    Application();
    
    /**
     * @brief Destructor
     */
    ~Application();
    
    /**
     * @brief Disable copy construction
     */
    Application(const Application&) = delete;
    
    /**
     * @brief Disable assignment
     */
    Application& operator=(const Application&) = delete;

    /**
     * @brief Run the application with command line arguments
     * 
     * This method initializes the application, parses command line arguments,
     * and executes the appropriate command.
     * 
     * @param argc Argument count
     * @param argv Argument values
     * @return int Return code (0 for success)
     */
    int Run(int argc, char* argv[]);
    
    /**
     * @brief Run with a specific command and arguments
     * 
     * This method is useful for testing and programmatic use of the application.
     * 
     * @param command Command name
     * @param args Command arguments
     * @return int Return code (0 for success)
     */
    int RunCommand(const std::string& command, const std::vector<std::string>& args);

private:
    // Forward declaration of implementation
    class Impl;
    
    // Private implementation (Pimpl pattern)
    std::unique_ptr<Impl> impl_;
};

} // namespace cortex