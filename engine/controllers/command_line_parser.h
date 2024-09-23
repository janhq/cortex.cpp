#pragma once

#include "CLI/CLI.hpp"
#include "services/engine_service.h"
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
                     std::string& version);

  void EngineUninstall(CLI::App* parent, const std::string& engine_name);

  void EngineGet(CLI::App* parent);

  CLI::App app_;
  EngineService engine_service_;
  struct CmlData{
    std::string model_id;
    std::string msg;
    std::string model_alias;
    std::string model_path;
    std::string engine_version = "latest";
    std::string cortex_version;
    bool check_upd = true;
    int port;
    config_yaml_utils::CortexConfig config;
  };
  CmlData cml_data_;
};
