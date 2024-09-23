#include "run_cmd.h"
#include "chat_cmd.h"
#include "cmd_info.h"
#include "config/yaml_config.h"
#include "model_start_cmd.h"
#include "model_status_cmd.h"
#include "server_start_cmd.h"
#include "utils/file_manager_utils.h"
#include "utils/modellist_utils.h"

namespace commands {

void RunCmd::Exec() {
  auto address = host_ + ":" + std::to_string(port_);
  CmdInfo ci(model_id_);
  std::string model_file =
      ci.branch == "main" ? ci.model_name : ci.model_name + "-" + ci.branch;
  // TODO should we clean all resource if something fails?
  // Check if model existed. If not, download it
  {
    auto model_conf = model_service_.GetDownloadedModel(model_file + ".yaml");
    if (!model_conf.has_value()) {
      model_service_.DownloadModel(model_id_);
    }
  }

  // Check if engine existed. If not, download it
  {
    auto required_engine = engine_service_.GetEngineInfo(ci.engine_name);
    if (!required_engine.has_value()) {
      throw std::runtime_error("Engine not found: " + ci.engine_name);
    }
    if (required_engine.value().status == EngineService::kIncompatible) {
      throw std::runtime_error("Engine " + ci.engine_name + " is incompatible");
    }
    if (required_engine.value().status == EngineService::kNotInstalled) {
      engine_service_.InstallEngine(ci.engine_name);
    }
  }

  // Start server if it is not running
  {
    if (!commands::IsServerAlive(host_, port_)) {
      CLI_LOG("Starting server ...");
      commands::ServerStartCmd ssc;
      if (!ssc.Exec(host_, port_)) {
        return;
      }
    }
  }

  // TODO(sang) refactor after `cortex pull` done with new data structure
  try {
    modellist_utils::ModelListUtils modellist_handler;
    config::YamlHandler yaml_handler;
    auto model_entry = modellist_handler.GetModelInfo(model_id_);
    yaml_handler.ModelConfigFromFile(model_entry.path_to_model_yaml);
    auto mc = yaml_handler.GetModelConfig();

    // Always start model if not llamacpp
    // If it is llamacpp, then check model status first
    {
      if ((mc.engine.find("llamacpp") == std::string::npos) ||
          !commands::ModelStatusCmd().IsLoaded(host_, port_, mc)) {
        if (!ModelStartCmd().Exec(host_, port_, mc)) {
          return;
        }
      }
    }

    // Chat
    ChatCmd().Exec(host_, port_, mc, "");
  } catch (const std::exception& e) {
    CLI_LOG("Fail to run model with ID '" + model_id_ + "': " + e.what());
  }
}
};  // namespace commands
