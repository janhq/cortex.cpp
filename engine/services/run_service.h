#pragma once

#include "common/run.h"
#include "utils/result.hpp"

class RunService {
 public:
  cpp::result<OpenAi::Run, std::string> CreateRun();
};
