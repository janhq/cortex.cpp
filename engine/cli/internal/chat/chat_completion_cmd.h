// commands/internal/chat/chat_completion_cmd.h
#pragma once

#include "public/command.h"
#include <string>
#include <vector>
#include <json/json.h>
#include "config/model_config.h"
#include "services/database_service.h"

namespace cortex {
namespace commands {

/**
 * @brief Command implementation for chat completions
 */
class ChatCompletionCommand : public Command {
public:
    /**
     * @brief Construct a new Chat Completion Command
     * 
     * @param db_service Database service for model information
     */
    explicit ChatCompletionCommand(std::shared_ptr<DatabaseService> db_service);

    /**
     * @brief Execute the chat command
     * 
     * @param args Positional arguments
     * @param options Command options
     * @return CommandStatus Execution status
     */
    CommandStatus Execute(
        const std::vector<std::string>& args,
        const std::unordered_map<std::string, std::string>& options) override;

    /**
     * @brief Get the command name
     * 
     * @return std::string Command name
     */
    std::string GetName() const override;

    /**
     * @brief Get the command description
     * 
     * @return std::string Command description
     */
    std::string GetDescription() const override;

    /**
     * @brief Get the command help text
     * 
     * @return std::string Help text
     */
    std::string GetHelp() const override;

private:
    // The original implementation uses these methods
    void Exec(const std::string& host, int port, 
              const std::string& model_handle, std::string msg);
              
    void Exec(const std::string& host, int port, 
              const std::string& model_handle,
              const config::ModelConfig& mc, std::string msg);

    std::shared_ptr<DatabaseService> db_service_;
    std::vector<Json::Value> histories_;
};

} // namespace commands
} // namespace cortex