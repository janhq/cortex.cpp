// commands/public/commands.h
#pragma once

#include "command.h"
#include "command_registry.h"

// Forward declarations of all command classes
namespace cortex {
namespace commands {

// Core commands
class ConfigGetCommand;
class ConfigUpdateCommand;
class ServerStartCommand;
class ServerStopCommand;
class HelpCommand;
class VersionCommand;
class PSCommand;

// Engine commands
class EngineGetCommand;
class EngineInstallCommand;
class EngineListCommand;
class EngineLoadCommand;
class EngineUninstallCommand;
class EngineUnloadCommand;
class EngineUpdateCommand;
class EngineUseCommand;

// Hardware commands
class HardwareActivateCommand;
class HardwareListCommand;

// Model commands
class ModelDeleteCommand;
class ModelGetCommand;
class ModelImportCommand;
class ModelListCommand;
class ModelPullCommand;
class ModelSourceAddCommand;
class ModelSourceDeleteCommand;
class ModelSourceListCommand;
class ModelStartCommand;
class ModelStatusCommand;
class ModelStopCommand;
class ModelUpdateCommand;

// Interaction commands
class ChatCompletionCommand;
class RunCommand;

/**
 * @brief Register all available commands with the registry
 * 
 * @param registry Command registry
 * @param service_provider Service provider with all required services
 */
void RegisterAllCommands(
    std::shared_ptr<CommandRegistry> registry,
    std::shared_ptr<ServiceProvider> service_provider);

/**
 * @brief Factory function to create a chat completion command
 * 
 * @param db_service Database service
 * @return std::shared_ptr<Command> Chat completion command
 */
std::shared_ptr<Command> CreateChatCompletionCommand(
    std::shared_ptr<DatabaseService> db_service);

/**
 * @brief Factory function to create a run command
 * 
 * @param db_service Database service
 * @param engine_service Engine service
 * @return std::shared_ptr<Command> Run command
 */
std::shared_ptr<Command> CreateRunCommand(
    std::shared_ptr<DatabaseService> db_service,
    std::shared_ptr<EngineService> engine_service);

// Add factory functions for all other commands
// ...

} // namespace commands
} // namespace cortex