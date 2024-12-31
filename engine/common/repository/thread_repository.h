#pragma once

#include "common/thread.h"
#include "utils/result.hpp"

class ThreadRepository {
 public:
  virtual cpp::result<void, std::string> CreateThread(
      const OpenAi::Thread& thread) = 0;

  virtual cpp::result<std::vector<OpenAi::Thread>, std::string> ListThreads(
      uint8_t limit, const std::string& order, const std::string&,
      const std::string& before) const = 0;

  virtual cpp::result<OpenAi::Thread, std::string> RetrieveThread(
      const std::string& thread_id) const = 0;

  virtual cpp::result<void, std::string> ModifyThread(
      OpenAi::Thread& thread) = 0;

  virtual cpp::result<void, std::string> DeleteThread(
      const std::string& thread_id) = 0;

  virtual ~ThreadRepository() = default;
};
