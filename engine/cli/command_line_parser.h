#pragma once

#include <memory>
#include "CLI/CLI.hpp"
#include "services/engine_service.h"
#include "services/model_service.h"
#include "utils/config_yaml_utils.h"

class CommandLineParser {
 public:
  CommandLineParser();
  bool SetupCommand(int argc, char** argv);

 private:
  void SetupCommonCommands();

  void SetupInferenceCommands();

  void SetupModelCommands();

  void SetupEngineCommands();

  void SetupSystemCommands();

  void EngineInstall(CLI::App* parent, const std::string& engine_name,
                     std::string& version, std::string& src);

  void EngineUninstall(CLI::App* parent, const std::string& engine_name);

  void EngineUpdate(CLI::App* parent, const std::string& engine_name);

  void EngineGet(CLI::App* parent);

  void EngineUse(CLI::App* parent, const std::string& engine_name);

  void ModelUpdate(CLI::App* parent);

  CLI::App app_;
  std::shared_ptr<DownloadService> download_service_;
  EngineService engine_service_;
  ModelService model_service_;

  struct CmlData {
    std::string model_id;
    std::string msg;
    std::string model_alias;
    std::string model_path;
    std::string engine_version = "latest";
    std::string engine_src;
    std::string cortex_version;
    bool check_upd = true;
    bool run_detach = false;

    // for model list
    bool display_engine = false;
    bool display_version = false;
    std::string filter = "";
    bool show_menu = false;

    int port;
    config_yaml_utils::CortexConfig config;
    std::unordered_map<std::string, std::string> model_update_options;
  };
  CmlData cml_data_;
  bool executed_ = false;
};
