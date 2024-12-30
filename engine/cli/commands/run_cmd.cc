#include "run_cmd.h"
#include "chat_completion_cmd.h"
#include "config/yaml_config.h"
#include "cortex_upd_cmd.h"
#include "database/models.h"
#include "engine_install_cmd.h"
#include "model_pull_cmd.h"
#include "model_start_cmd.h"
#include "model_status_cmd.h"
#include "server_start_cmd.h"
#include "utils/cli_selection_utils.h"
#include "utils/logging_utils.h"

namespace commands {

std::optional<std::string> SelectLocalModel(std::string host, int port,
                                            const std::string& model_handle,
                                            DatabaseService& db_service) {
  std::optional<std::string> model_id = model_handle;
  if (model_handle.empty()) {
    auto all_local_models = db_service.LoadModelList();
    if (all_local_models.has_error() || all_local_models.value().empty()) {
      CLI_LOG("No local models available!");
      return std::nullopt;
    }

    if (all_local_models.value().size() == 1) {
      model_id = all_local_models.value().front().model;
    } else {
      std::vector<std::string> model_id_list{};
      for (const auto& model : all_local_models.value()) {
        model_id_list.push_back(model.model);
      }

      auto selection = cli_selection_utils::PrintSelection(
          model_id_list, "Please select an option");
      if (!selection.has_value()) {
        return std::nullopt;
      }
      model_id = selection.value();
      CLI_LOG("Selected: " << selection.value());
    }
  } else {
    auto related_models_ids = db_service.FindRelatedModel(model_handle);
    if (related_models_ids.has_error() || related_models_ids.value().empty()) {
      auto result = ModelPullCmd().Exec(host, port, model_handle);
      if (!result) {
        CLI_LOG("Model " << model_handle << " not found!");
        return std::nullopt;
      }
      model_id = result.value();
      CTL_INF("model_id: " << model_id.value());
    } else if (related_models_ids.value().size() == 1) {
      model_id = related_models_ids.value().front();
    } else {  // multiple models with nearly same name found
      auto selection = cli_selection_utils::PrintSelection(
          related_models_ids.value(), "Local Models: (press enter to select)");
      if (!selection.has_value()) {
        return std::nullopt;
      }
      model_id = selection.value();
      CLI_LOG("Selected: " << selection.value());
    }
  }
  return model_id;
}

void RunCmd::Exec(bool run_detach,
                  const std::unordered_map<std::string, std::string>& options) {
  std::optional<std::string> model_id =
      SelectLocalModel(host_, port_, model_handle_, *db_service_);
  if (!model_id.has_value()) {
    return;
  }

  config::YamlHandler yaml_handler;
  auto address = host_ + ":" + std::to_string(port_);

  try {
    namespace fs = std::filesystem;
    namespace fmu = file_manager_utils;
    auto model_entry = db_service_->GetModelInfo(*model_id);
    if (model_entry.has_error()) {
      CLI_LOG("Error: " + model_entry.error());
      return;
    }
    yaml_handler.ModelConfigFromFile(
        fmu::ToAbsoluteCortexDataPath(
            fs::path(model_entry.value().path_to_model_yaml))
            .string());
    auto mc = yaml_handler.GetModelConfig();

    // Check if engine existed. If not, download it
    {
      auto is_engine_ready = engine_service_->IsEngineReady(mc.engine);
      if (is_engine_ready.has_error()) {
        throw std::runtime_error(is_engine_ready.error());
      }

      if (!is_engine_ready.value()) {
        CTL_INF("Engine " << mc.engine
                          << " is not ready. Proceed to install..");
        if (!EngineInstallCmd(engine_service_, host_, port_, false)
                 .Exec(mc.engine)) {
          return;
        } else {
          CTL_INF("Engine " << mc.engine << " is ready");
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
        if ((mc.engine.find(kLlamaRepo) == std::string::npos &&
             mc.engine.find(kLlamaEngine) == std::string::npos) ||
            !commands::ModelStatusCmd().IsLoaded(host_, port_, *model_id)) {

          auto res = commands::ModelStartCmd(db_service_)
                         .Exec(host_, port_, *model_id, options,
                               false /*print_success_log*/);
          if (!res) {
            CLI_LOG("Error: Failed to start model");
            return;
          }
        }
      }

      // Chat
      if (run_detach) {
        CLI_LOG(*model_id << " model started successfully. Use `"
                          << commands::GetCortexBinary() << " run " << *model_id
                          << "` for interactive chat shell");
      } else {
        ChatCompletionCmd(db_service_).Exec(host_, port_, *model_id, mc, "");
      }
    }
  } catch (const std::exception& e) {
    CLI_LOG("Fail to run model with ID '" + model_handle_ + "': " + e.what());
  }
}
};  // namespace commands
