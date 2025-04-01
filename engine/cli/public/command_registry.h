// commands/public/command_registry.h
#pragma once

#include <memory>
#include <string>
#include <vector>
#include "command.h"

namespace cortex {
namespace commands {

/**
 * @brief Registry for CLI commands
 * 
 * This class manages a collection of command implementations and provides
 * lookup functionality.
 */
class CommandRegistry {
public:
    /**
     * @brief Create a new command registry
     * 
     * @return std::shared_ptr<CommandRegistry> New command registry instance
     */
    static std::shared_ptr<CommandRegistry> Create();

    virtual ~CommandRegistry() = default;

    /**
     * @brief Register a command
     * 
     * @param command Command implementation to register
     * @return bool True if registration was successful
     */
    virtual bool RegisterCommand(std::shared_ptr<Command> command) = 0;

    /**
     * @brief Find a command by name
     * 
     * @param name Command name to find
     * @return std::shared_ptr<Command> Command if found, nullptr otherwise
     */
    virtual std::shared_ptr<Command> FindCommand(const std::string& name) const = 0;

    /**
     * @brief Get all registered commands
     * 
     * @return std::vector<std::shared_ptr<Command>> List of all commands
     */
    virtual std::vector<std::shared_ptr<Command>> GetAllCommands() const = 0;

    /**
     * @brief Execute a command by name
     * 
     * @param command_name Command name
     * @param args Command arguments
     * @param options Command options
     * @return CommandStatus Execution status
     */
    virtual CommandStatus ExecuteCommand(
        const std::string& command_name,
        const std::vector<std::string>& args,
        const std::unordered_map<std::string, std::string>& options) = 0;

    /**
     * @brief Print help for all commands
     */
    virtual void PrintHelp() const = 0;

    /**
     * @brief Print help for a specific command
     * 
     * @param command_name Command name
     * @return bool True if command was found and help printed
     */
    virtual bool PrintCommandHelp(const std::string& command_name) const = 0;
};

} // namespace commands
} // namespace cortex