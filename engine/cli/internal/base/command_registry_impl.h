#pragma once

#include "public/command_registry.h"
#include <unordered_map>

namespace cortex {
namespace commands {

/**
 * @brief Implementation of command registry
 */
class CommandRegistryImpl : public CommandRegistry {
public:
    CommandRegistryImpl() = default;
    
    bool RegisterCommand(std::shared_ptr<Command> command) override;
    
    std::shared_ptr<Command> FindCommand(const std::string& name) const override;
    
    std::vector<std::shared_ptr<Command>> GetAllCommands() const override;
    
    CommandStatus ExecuteCommand(
        const std::string& command_name,
        const std::vector<std::string>& args,
        const std::unordered_map<std::string, std::string>& options) override;
    
    void PrintHelp() const override;
    
    bool PrintCommandHelp(const std::string& command_name) const override;

private:
    std::unordered_map<std::string, std::shared_ptr<Command>> commands_;
};

} // namespace commands
} // namespace cortex