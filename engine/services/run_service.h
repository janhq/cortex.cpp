#pragma once

#include "common/dto/run_create_dto.h"
#include "common/events/assistant_stream_event.h"
#include "common/run.h"
#include "services/assistant_service.h"
#include "services/inference_service.h"
#include "services/message_service.h"
#include "services/model_service.h"
#include "services/thread_service.h"
#include "utils/result.hpp"

class RunService {
 public:
  auto CreateRun(const dto::RunCreateDto& create_dto)
      -> cpp::result<OpenAi::Run, std::string>;

  auto CreateRunStream(
      dto::RunCreateDto&& create_dto, const std::string& thread_id,
      std::function<void(const OpenAi::AssistantStreamEvent&, bool disconnect)>
          callback) -> void;

  explicit RunService(std::shared_ptr<AssistantService> assistant_srv,
                      std::shared_ptr<ModelService> model_srv,
                      std::shared_ptr<MessageService> message_srv,
                      std::shared_ptr<InferenceService> inference_srv,
                      std::shared_ptr<ThreadService> thread_srv)
      : assistant_srv_{assistant_srv},
        model_srv_{model_srv},
        message_srv_{message_srv},
        inference_srv_{inference_srv},
        thread_srv_{thread_srv} {}

 private:
  std::shared_ptr<AssistantService> assistant_srv_;
  std::shared_ptr<ModelService> model_srv_;
  std::shared_ptr<MessageService> message_srv_;
  std::shared_ptr<InferenceService> inference_srv_;
  std::shared_ptr<ThreadService> thread_srv_;
};
