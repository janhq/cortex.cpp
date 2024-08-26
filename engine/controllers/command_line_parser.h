#pragma once

#include <vector>
#include "CLI/CLI.hpp"

class CommandLineParser {
 public:
  CommandLineParser();
  bool SetupCommand(int argc, char** argv);

 private:
  CLI::App app_;
};