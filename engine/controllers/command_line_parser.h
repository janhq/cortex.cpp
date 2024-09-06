#pragma once

#include "CLI/CLI.hpp"
#include "services/engine_service.h"

class CommandLineParser {
 public:
  CommandLineParser();
  bool SetupCommand(int argc, char** argv);

 private:
  void EngineManagement(CLI::App* parent, const std::string& engine_name,
                        std::string& version);

  void EngineGet(CLI::App* parent);

  CLI::App app_;
  EngineService engine_service_;
};
