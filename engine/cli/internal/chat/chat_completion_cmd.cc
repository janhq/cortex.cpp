// commands/internal/chat/chat_completion_cmd.cc
#include "chat_completion_cmd.h"
#include <curl/curl.h>
#include "config/yaml_config.h"
#include "model_status_cmd.h"
#include "server_start_cmd.h"
#include "utils/engine_constants.h"
#include "utils/logging_utils.h"
#include "utils/string_utils.h"

// For convenience, using the existing commands namespace
using namespace commands;

namespace cortex {
namespace commands {

ChatCompletionCommand::ChatCompletionCommand(std::shared_ptr<DatabaseService> db_service)
    : db_service_(std::move(db_service)) {}

CommandStatus ChatCompletionCommand::Execute(
    const std::vector<std::string>& args,
    const std::unordered_map<std::string, std::string>& options) {
  
  // Extract host and port from options (or use defaults)
  std::string host = "127.0.0.1";
  int port = 39281;
  
  auto it = options.find("host");
  if (it != options.end()) {
    host = it->second;
  }
  
  it = options.find("port");
  if (it != options.end()) {
    try {
      port = std::stoi(it->second);
    } catch (...) {
      std::cerr << "Invalid port number: " << it->second << std::endl;
      return CommandStatus::InvalidArguments;
    }
  }
  
  // Extract model handle (from args or options)
  std::string model_handle;
  if (!args.empty()) {
    model_handle = args[0];
  } else {
    it = options.find("model");
    if (it != options.end()) {
      model_handle = it->second;
    } else {
      std::cerr << "Model handle is required" << std::endl;
      return CommandStatus::InvalidArguments;
    }
  }
  
  // Extract message (if any)
  std::string msg;
  it = options.find("message");
  if (it != options.end()) {
    msg = it->second;
  }
  
  // Call the original implementation
  try {
    Exec(host, port, model_handle, msg);
    return CommandStatus::Success;
  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return CommandStatus::GeneralError;
  }
}

std::string ChatCompletionCommand::GetName() const {
  return "chat";
}

std::string ChatCompletionCommand::GetDescription() const {
  return "Chat with a model interactively or send a single message";
}

std::string ChatCompletionCommand::GetHelp() const {
  return "Usage: cortex chat [options] MODEL_ID\n"
         "\n"
         "Start a chat session with the specified model.\n"
         "\n"
         "Arguments:\n"
         "  MODEL_ID             Model ID to chat with\n"
         "\n"
         "Options:\n"
         "  --message=MESSAGE    Initial message (optional, interactive mode if not provided)\n"
         "  --host=HOST          Server host (default: 127.0.0.1)\n"
         "  --port=PORT          Server port (default: 39281)\n";
}

// Reusing the original implementation
void ChatCompletionCommand::Exec(const std::string& host, int port,
                             const std::string& model_handle, std::string msg) {
  // Reuse the existing implementation
  ::commands::ChatCompletionCmd original_cmd(db_service_);
  original_cmd.Exec(host, port, model_handle, msg);
}

void ChatCompletionCommand::Exec(const std::string& host, int port,
                             const std::string& model_handle,
                             const config::ModelConfig& mc, std::string msg) {
  // Reuse the existing implementation
  ::commands::ChatCompletionCmd original_cmd(db_service_);
  original_cmd.Exec(host, port, model_handle, mc, msg);
}

} // namespace commands
} // namespace cortex