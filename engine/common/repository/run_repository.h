#pragma once

#include "common/run.h"
#include "utils/result.hpp"

class RunRepository {
 public:
  virtual cpp::result<void, std::string> CreateRun(const OpenAi::Run& run) = 0;

  virtual cpp::result<std::vector<OpenAi::Run>, std::string> ListRuns(
      uint8_t limit, const std::string& order, const std::string& after,
      const std::string& before) const = 0;

  virtual cpp::result<OpenAi::Run, std::string> RetrieveRun(
      const std::string& run_id) const = 0;

  virtual cpp::result<void, std::string> ModifyRun(OpenAi::Run& run) = 0;

  virtual cpp::result<void, std::string> DeleteRun(
      const std::string& run_id) = 0;

  virtual ~RunRepository() = default;
};
