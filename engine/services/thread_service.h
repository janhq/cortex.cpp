#pragma once

#include <optional>
#include "common/repository/thread_repository.h"
#include "common/variant_map.h"
#include "utils/result.hpp"

class ThreadService {
 public:
  explicit ThreadService(std::shared_ptr<ThreadRepository> thread_repository)
      : thread_repository_{thread_repository} {}

  cpp::result<OpenAi::Thread, std::string> CreateThread(
      std::unique_ptr<OpenAi::ToolResources> tool_resources,
      std::optional<Cortex::VariantMap> metadata);

  cpp::result<std::vector<OpenAi::Thread>, std::string> ListThreads(
      uint8_t limit, const std::string& order, const std::string& after,
      const std::string& before) const;

  cpp::result<OpenAi::Thread, std::string> RetrieveThread(
      const std::string& thread_id) const;

  cpp::result<OpenAi::Thread, std::string> ModifyThread(
      const std::string& thread_id,
      std::unique_ptr<OpenAi::ToolResources> tool_resources,
      std::optional<Cortex::VariantMap> metadata);

  cpp::result<std::string, std::string> DeleteThread(
      const std::string& thread_id);

 private:
  std::shared_ptr<ThreadRepository> thread_repository_;
};
