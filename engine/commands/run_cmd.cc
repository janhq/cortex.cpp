#include "run_cmd.h"
#include "chat_completion_cmd.h"
#include "config/yaml_config.h"
#include "database/models.h"
#include "model_start_cmd.h"
#include "model_status_cmd.h"
#include "server_start_cmd.h"
#include "utils/logging_utils.h"

#include "cortex_upd_cmd.h"

namespace commands {

void RunCmd::Exec(bool chat_flag) {
  std::optional<std::string> model_id = model_handle_;

  cortex::db::Models modellist_handler;
  config::YamlHandler yaml_handler;
  auto address = host_ + ":" + std::to_string(port_);

  // Download model if it does not exist
  {
    if (!modellist_handler.HasModel(model_handle_)) {
      auto result = model_service_.DownloadModel(model_handle_);
      if (result.has_error()) {
        CTL_ERR("Error: " << result.error());
        return;
      }
      model_id = result.value();
      CTL_INF("model_id: " << model_id.value());
    }
  }

  try {
    auto model_entry = modellist_handler.GetModelInfo(*model_id);
    if (model_entry.has_error()) {
      CLI_LOG("Error: " + model_entry.error());
      return;
    }
    yaml_handler.ModelConfigFromFile(model_entry.value().path_to_model_yaml);
    auto mc = yaml_handler.GetModelConfig();

    // Check if engine existed. If not, download it
    {
      auto required_engine = engine_service_.GetEngineInfo(mc.engine);
      if (!required_engine.has_value()) {
        throw std::runtime_error("Engine not found: " + mc.engine);
      }
      if (required_engine.value().status == EngineService::kIncompatible) {
        throw std::runtime_error("Engine " + mc.engine + " is incompatible");
      }
      if (required_engine.value().status == EngineService::kNotInstalled) {
        auto install_engine_result = engine_service_.InstallEngine(mc.engine);
        if (install_engine_result.has_error()) {
          throw std::runtime_error(install_engine_result.error());
        }
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
          !commands::ModelStatusCmd().IsLoaded(host_, port_, *model_id)) {
        if (!ModelStartCmd().Exec(host_, port_, *model_id)) {
          return;
        }
      }
    }

    // Chat
    if (chat_flag) {
      ChatCompletionCmd().Exec(host_, port_, *model_id, mc, "");
    } else {
      CLI_LOG(*model_id << " model started successfully. Use `"
                        << commands::GetCortexBinary() << " chat " << *model_id
                        << "` for interactive chat shell");
    }
  } catch (const std::exception& e) {
    CLI_LOG("Fail to run model with ID '" + model_handle_ + "': " + e.what());
  }
}
};  // namespace commands
