#pragma once

#include <vector>
#include "CLI/CLI.hpp"

class CommandLineParser {
 public:
  CommandLineParser();
  bool SetupCommand(int argc, char** argv);

 private:
  void EngineInstall(CLI::App* parent, const std::string& engine_name);

  CLI::App app_;
};