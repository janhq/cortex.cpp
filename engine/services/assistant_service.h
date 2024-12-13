#pragma once

#include "common/assistant.h"
#include "repositories/thread_fs_repository.h"
#include "utils/result.hpp"

class AssistantService {
 public:
  explicit AssistantService(
      std::shared_ptr<AssistantBackwardCompatibleSupport> thread_repository)
      : thread_repository_{thread_repository} {}

  cpp::result<OpenAi::JanAssistant, std::string> CreateAssistant(
      const std::string& thread_id, const OpenAi::JanAssistant& assistant);

  cpp::result<OpenAi::JanAssistant, std::string> RetrieveAssistant(
      const std::string& thread_id) const;

  cpp::result<OpenAi::JanAssistant, std::string> ModifyAssistant(
      const std::string& thread_id, const OpenAi::JanAssistant& assistant);

 private:
  std::shared_ptr<AssistantBackwardCompatibleSupport> thread_repository_;
};
