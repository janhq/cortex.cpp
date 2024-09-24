#include "run_cmd.h"
#include "chat_cmd.h"
#include "cmd_info.h"
#include "config/yaml_config.h"
#include "model_start_cmd.h"
#include "model_status_cmd.h"
#include "server_start_cmd.h"
#include "utils/file_manager_utils.h"
#include "utils/modellist_utils.h"
#include "utils/cortex_utils.h"
namespace commands {

void RunCmd::Exec() {
  auto model_id = cortex_utils::GetModelIdFromHandle(model_handle_);

  if (!model_id.has_value()) {
    CTL_ERR("Could not get model_id from handle: " << model_handle_);
    return;
  }
  modellist_utils::ModelListUtils modellist_handler;
  config::YamlHandler yaml_handler;
  auto address = host_ + ":" + std::to_string(port_);

// Download model if it does not exist
  {
    if (!modellist_handler.HasModel(*model_id)) {
      model_service_.DownloadModel(model_handle_);
    }
  }

  try {
    auto model_entry = modellist_handler.GetModelInfo(*model_id);
    yaml_handler.ModelConfigFromFile(model_entry.path_to_model_yaml);
    auto mc = yaml_handler.GetModelConfig();

    // Check if engine existed. If not, download it
    {
      auto required_engine = engine_service_.GetEngineInfo(mc.engine);
      if (!required_engine.has_value()) {
        throw std::runtime_error("Engine not found: " + mc.engine);
      }
      if (required_engine.value().status == EngineService::kIncompatible) {
        throw std::runtime_error("Engine " + mc.engine +
                                 " is incompatible");
      }
      if (required_engine.value().status == EngineService::kNotInstalled) {
        engine_service_.InstallEngine(mc.engine);
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
    CLI_LOG("Fail to run model with ID '" + model_handle_ + "': " + e.what());
  }
}
};  // namespace commands
