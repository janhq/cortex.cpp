#pragma once

#include "CLI/CLI.hpp"
#include "services/engine_service.h"

class CommandLineParser {
 public:
  CommandLineParser();
  bool SetupCommand(int argc, char** argv);

 private:
  void EngineInstall(CLI::App* parent, const std::string& engine_name,
                     std::string& version, std::string& src);

  void EngineUninstall(CLI::App* parent, const std::string& engine_name);

  void EngineGet(CLI::App* parent);

  CLI::App app_;
  EngineService engine_service_;
};
