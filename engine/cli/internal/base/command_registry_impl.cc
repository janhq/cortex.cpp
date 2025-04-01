#include "command_registry_impl.h"
#include <algorithm>
#include <iostream>

namespace cortex {
namespace commands {

bool CommandRegistryImpl::RegisterCommand(std::shared_ptr<Command> command) {
    if (!command) {
        return false;
    }
    
    const auto& name = command->GetName();
    if (name.empty() || commands_.find(name) != commands_.end()) {
        return false;
    }
    
    commands_[name] = std::move(command);
    return true;
}

std::shared_ptr<Command> CommandRegistryImpl::FindCommand(const std::string& name) const {
    auto it = commands_.find(name);
    if (it != commands_.end()) {
        return it->second;
    }
    return nullptr;
}

std::vector<std::shared_ptr<Command>> CommandRegistryImpl::GetAllCommands() const {
    std::vector<std::shared_ptr<Command>> result;
    result.reserve(commands_.size());
    
    for (const auto& [_, command] : commands_) {
        result.push_back(command);
    }
    
    // Sort commands by name for consistent output
    std::sort(result.begin(), result.end(), 
        [](const std::shared_ptr<Command>& a, const std::shared_ptr<Command>& b) {
            return a->GetName() < b->GetName();
        });
    
    return result;
}

CommandStatus CommandRegistryImpl::ExecuteCommand(
    const std::string& command_name,
    const std::vector<std::string>& args,
    const std::unordered_map<std::string, std::string>& options) {
    
    auto command = FindCommand(command_name);
    if (!command) {
        std::cerr << "Unknown command: " << command_name << std::endl;
        PrintHelp();
        return CommandStatus::NotFound;
    }
    
    return command->Execute(args, options);
}

void CommandRegistryImpl::PrintHelp() const {
    std::cout << "Cortex Command Line Interface" << std::endl;
    std::cout << "Usage: cortex [command] [options]" << std::endl;
    std::cout << std::endl;
    
    std::cout << "Available commands:" << std::endl;
    
    auto all_commands = GetAllCommands();
    size_t max_name_length = 0;
    
    // Find the longest command name for alignment
    for (const auto& command : all_commands) {
        max_name_length = std::max(max_name_length, command->GetName().length());
    }
    
    // Print all commands
    for (const auto& command : all_commands) {
        std::cout << "  " << command->GetName();
        
        // Add padding for alignment
        for (size_t i = command->GetName().length(); i < max_name_length + 2; ++i) {
            std::cout << " ";
        }
        
        std::cout << command->GetDescription() << std::endl;
    }
    
    std::cout << std::endl;
    std::cout << "For more information about a specific command, type 'cortex help [command]'" << std::endl;
}

bool CommandRegistryImpl::PrintCommandHelp(const std::string& command_name) const {
    auto command = FindCommand(command_name);
    if (!command) {
        std::cout << "Unknown command: " << command_name << std::endl;
        return false;
    }
    
    std::cout << command->GetHelp() << std::endl;
    return true;
}

// Static factory method implementation
std::shared_ptr<CommandRegistry> CommandRegistry::Create() {
    return std::make_shared<CommandRegistryImpl>();
}

} // namespace commands
} // namespace cortex
