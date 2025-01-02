#pragma once

#include "common/dto/run_create_dto.h"
#include "common/run.h"
#include "services/assistant_service.h"
#include "services/message_service.h"
#include "services/model_service.h"
#include "utils/result.hpp"

class RunService {
 public:
  auto CreateRun(const dto::RunCreateDto& create_dto)
      -> cpp::result<OpenAi::Run, std::string>;

  auto CreateRunStream(
      const dto::RunCreateDto& create_dto,
      std::function<void(const std::string&, bool is_done)> callback) -> void;

  explicit RunService(std::shared_ptr<AssistantService> assistant_srv,
                      std::shared_ptr<ModelService> model_srv,
                      std::shared_ptr<MessageService> message_srv)
      : assistant_srv_{assistant_srv},
        model_srv_{model_srv},
        message_srv_{message_srv} {}

 private:
  std::shared_ptr<AssistantService> assistant_srv_;
  std::shared_ptr<ModelService> model_srv_;
  std::shared_ptr<MessageService> message_srv_;
};
