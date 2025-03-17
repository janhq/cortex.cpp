#pragma once

#include <memory>
#include <unordered_map>
#include "CLI/CLI.hpp"
#include "commands/hardware_list_cmd.h"
#include "services/engine_service.h"
#include "utils/config_yaml_utils.h"

class CommandLineParser {
 public:
  CommandLineParser();
  bool SetupCommand(int argc, char** argv);

 private:
  void SetupCommonCommands();

  void SetupModelCommands();

  void SetupEngineCommands();

  void SetupHardwareCommands();

  void SetupSystemCommands();

  void SetupConfigsCommands();

  void EngineUpdate(CLI::App* parent, const std::string& engine_name);

  void EngineGet(CLI::App* parent);

  void EngineUse(CLI::App* parent, const std::string& engine_name);

  void EngineLoad(CLI::App* parent, const std::string& engine_name);

  void EngineUnload(CLI::App* parent, const std::string& engine_name);

  void ModelUpdate(CLI::App* parent);

  CLI::App app_;
  std::shared_ptr<DownloadService> download_service_;
  std::shared_ptr<cortex::DylibPathManager> dylib_path_manager_;
  std::shared_ptr<DatabaseService> db_service_;
  std::shared_ptr<EngineService> engine_service_;
  std::vector<std::string> supported_engines_;

  struct CmlData {
    std::string model_id;
    std::string msg;
    std::string model_alias;
    std::string model_path;
    std::string engine_name;
    std::string engine_version = "latest";
    std::string engine_src;
    std::string cortex_version;
    bool check_upd = true;
    bool run_detach = false;

    // for model list
    bool display_engine = false;
    bool display_version = false;
    bool display_cpu_mode = false;
    bool display_gpu_mode = false;
    bool display_available_model = false;
    std::string filter = "";
    std::string log_level = "INFO";

    bool show_menu = false;

    int port;
    config_yaml_utils::CortexConfig config;
    std::unordered_map<std::string, std::string> model_update_options;
    std::string model_src;
  };
  CmlData cml_data_;
  std::unordered_map<std::string, std::string> config_update_opts_;
  bool executed_ = false;
  commands::HarwareOptions hw_opts_;
  std::unordered_map<std::string, std::string> run_settings_;
};
