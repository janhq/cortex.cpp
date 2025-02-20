#pragma once

#include "common/assistant.h"
#include "utils/result.hpp"

class AssistantRepository {
 public:
  virtual cpp::result<std::vector<OpenAi::Assistant>, std::string>
  ListAssistants(uint8_t limit, const std::string& order,
                 const std::string& after, const std::string& before) const = 0;

  virtual cpp::result<OpenAi::Assistant, std::string> CreateAssistant(
      OpenAi::Assistant& assistant) = 0;

  virtual cpp::result<OpenAi::Assistant, std::string> RetrieveAssistant(
      const std::string assistant_id) const = 0;

  virtual cpp::result<void, std::string> ModifyAssistant(
      OpenAi::Assistant& assistant) = 0;

  virtual cpp::result<void, std::string> DeleteAssistant(
      const std::string& assitant_id) = 0;

  virtual ~AssistantRepository() = default;
};
