// commands/public/command.h
#pragma once

#include <string>
#include <vector>
#include <unordered_map>

namespace cortex {
namespace commands {

/**
 * @brief Status codes for command execution
 */
enum class CommandStatus {
    GENERAL_ERROR = -1,
    GENERAL_SUCCESS = 1,
};

/**
 * @brief Base interface for all CLI commands
 * 
 * This abstract class defines the interface that all command implementations
 * must follow. Commands can have arguments, options, and execute operations.
 */
class Command {
public:
    virtual ~Command() = default;

    /**
     * @brief Execute the command with the given arguments and options
     * 
     * @param args Positional arguments for the command
     * @param options Key-value pairs for command options
     * @return CommandStatus Return status code
     */
    virtual CommandStatus Execute(
        const std::vector<std::string>& args,
        const std::unordered_map<std::string, std::string>& options) = 0;

    /**
     * @brief Get the name of the command
     * 
     * @return std::string Command name
     */
    virtual std::string GetName() const = 0;

    /**
     * @brief Get a description of the command
     * 
     * @return std::string Command description
     */
    virtual std::string GetDescription() const = 0;

    /**
     * @brief Get help information for the command
     * 
     * @return std::string Help text
     */
    virtual std::string GetHelp() const = 0;
};

} // namespace commands
} // namespace cortex