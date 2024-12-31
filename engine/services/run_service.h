#pragma once

#include "common/dto/run_create_dto.h"
#include "common/dto/run_update_dto.h"
#include "common/events/assistant_stream_event.h"
#include "common/repository/run_repository.h"
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

  /**
   * Retrieves a run.
   */
  auto RetrieveRun(const std::string& thread_id,
                   const std::string& run_id) const
      -> cpp::result<OpenAi::Run, std::string>;

  /**
   * Cancels a run that is in_progress.
   */
  auto CancelRun(const std::string& thread_id, const std::string& run_id)
      -> cpp::result<OpenAi::Run, std::string>;

  /**
   * Modifies a run.
   */
  auto ModifyRun(const std::string& thread_id, const std::string& run_id,
                 const dto::RunUpdateDto& update_dto) -> void;

  /**
   * Returns a list of runs belonging to a thread.
   */
  auto ListRuns(const std::string& thread_id, uint8_t limit,
                const std::string& order, const std::string& after,
                const std::string& before) const
      -> cpp::result<std::vector<OpenAi::Run>, std::string>;

  explicit RunService(std::shared_ptr<RunRepository> run_repo,
                      std::shared_ptr<AssistantService> assistant_srv,
                      std::shared_ptr<ModelService> model_srv,
                      std::shared_ptr<MessageService> message_srv,
                      std::shared_ptr<InferenceService> inference_srv,
                      std::shared_ptr<ThreadService> thread_srv)
      : run_repository_{run_repo},
        assistant_srv_{assistant_srv},
        model_srv_{model_srv},
        message_srv_{message_srv},
        inference_srv_{inference_srv},
        thread_srv_{thread_srv} {}

 private:
  std::shared_ptr<RunRepository> run_repository_;

  std::shared_ptr<AssistantService> assistant_srv_;
  std::shared_ptr<ModelService> model_srv_;
  std::shared_ptr<MessageService> message_srv_;
  std::shared_ptr<InferenceService> inference_srv_;
  std::shared_ptr<ThreadService> thread_srv_;

  auto GetMessageListAsJson(const std::string& thread_id)
      -> cpp::result<std::unique_ptr<Json::Value>, std::string>;
};
