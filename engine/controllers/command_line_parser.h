#pragma once

#include "CLI/CLI.hpp"

class CommandLineParser {
 public:
  CommandLineParser();
  bool SetupCommand(int argc, char** argv);

 private:
  void EngineInstall(CLI::App* parent, const std::string& engine_name,
                     std::string& version);

  void SysInfo(CLI::App* parent);

  CLI::App app_;
};
