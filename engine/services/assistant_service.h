#pragma once

#include "common/assistant.h"
#include "common/dto/assistant_create_dto.h"
#include "common/dto/assistant_update_dto.h"
#include "common/repository/assistant_repository.h"
#include "repositories/thread_fs_repository.h"
#include "utils/result.hpp"

class AssistantService {
 public:
  cpp::result<OpenAi::JanAssistant, std::string> CreateAssistant(
      const std::string& thread_id, const OpenAi::JanAssistant& assistant);

  cpp::result<OpenAi::JanAssistant, std::string> RetrieveAssistant(
      const std::string& thread_id) const;

  cpp::result<OpenAi::JanAssistant, std::string> ModifyAssistant(
      const std::string& thread_id, const OpenAi::JanAssistant& assistant);

  // V2
  cpp::result<OpenAi::Assistant, std::string> CreateAssistantV2(
      const dto::CreateAssistantDto& create_dto);

  cpp::result<std::vector<OpenAi::Assistant>, std::string> ListAssistants(
      uint8_t limit, const std::string& order, const std::string& after,
      const std::string& before) const;

  cpp::result<OpenAi::Assistant, std::string> RetrieveAssistantV2(
      const std::string& assistant_id) const;

  cpp::result<OpenAi::Assistant, std::string> ModifyAssistantV2(
      const std::string& assistant_id,
      const dto::UpdateAssistantDto& update_dto);

  cpp::result<void, std::string> DeleteAssistantV2(
      const std::string& assistant_id);

  explicit AssistantService(
      std::shared_ptr<AssistantBackwardCompatibleSupport> thread_repository,
      std::shared_ptr<AssistantRepository> assistant_repository)
      : thread_repository_{thread_repository},
        assistant_repository_{assistant_repository} {}

 private:
  std::shared_ptr<AssistantBackwardCompatibleSupport> thread_repository_;
  std::shared_ptr<AssistantRepository> assistant_repository_;
};
