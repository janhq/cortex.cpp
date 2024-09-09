#include "run_cmd.h"
#include "chat_cmd.h"
#include "cmd_info.h"
#include "config/yaml_config.h"
#include "engine_init_cmd.h"
#include "model_pull_cmd.h"
#include "model_start_cmd.h"
#include "trantor/utils/Logger.h"
#include "utils/cortex_utils.h"
#include "utils/file_manager_utils.h"

namespace commands {

RunCmd::RunCmd(std::string host, int port, std::string model_id)
    : host_(std::move(host)), port_(port), model_id_(std::move(model_id)) {}

void RunCmd::Exec() {
  auto address = host_ + ":" + std::to_string(port_);
  CmdInfo ci(model_id_);
  std::string model_file =
      ci.branch == "main" ? ci.model_name : ci.model_name + "-" + ci.branch;
  // TODO should we clean all resource if something fails?
  // Check if model existed. If not, download it
  {
    if (!IsModelExisted(model_file)) {
      ModelPullCmd model_pull_cmd(ci.model_name, ci.branch);
      if (!model_pull_cmd.Exec()) {
        return;
      }
    }
  }

  // Check if engine existed. If not, download it
  {
    if (!IsEngineExisted(ci.engine_name)) {
      EngineInitCmd eic(ci.engine_name, "");
      if (!eic.Exec()) {
        LOG_INFO << "Failed to install engine";
        return;
      }
    }
  }

  // Start model
  config::YamlHandler yaml_handler;
  yaml_handler.ModelConfigFromFile(
      file_manager_utils::GetModelsContainerPath().string() + "/" + model_file +
      ".yaml");
  {
    ModelStartCmd msc(host_, port_, yaml_handler.GetModelConfig());
    if (!msc.Exec()) {
      return;
    }
  }

  // Chat
  {
    ChatCmd cc(host_, port_, yaml_handler.GetModelConfig());
    cc.Exec("");
  }
}

bool RunCmd::IsModelExisted(const std::string& model_id) {
  auto models_path = file_manager_utils::GetModelsContainerPath();
  if (std::filesystem::exists(models_path) &&
      std::filesystem::is_directory(models_path)) {
    // Iterate through directory
    for (const auto& entry : std::filesystem::directory_iterator(models_path)) {
      if (entry.is_regular_file() && entry.path().extension() == ".yaml") {
        try {
          config::YamlHandler handler;
          handler.ModelConfigFromFile(entry.path().string());
          if (entry.path().stem().string() == model_id) {
            return true;
          }
        } catch (const std::exception& e) {
          LOG_ERROR << "Error reading yaml file '" << entry.path().string()
                    << "': " << e.what();
        }
      }
    }
  }
  return false;
}

bool RunCmd::IsEngineExisted(const std::string& e) {
  auto engines_path = file_manager_utils::GetEnginesContainerPath();
  if (std::filesystem::exists(engines_path) &&
      std::filesystem::exists(engines_path.string() + "/" + e)) {
    return true;
  }
  return false;
}

};  // namespace commands
