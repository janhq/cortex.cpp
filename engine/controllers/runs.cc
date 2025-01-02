#include "runs.h"
#include <memory>
#include "common/cortex/sync_queue.h"
#include "common/events/assistant_stream_event.h"
#include "utils/cortex_utils.h"
#include "utils/logging_utils.h"

void Runs::CreateRun(const HttpRequestPtr& req,
                     std::function<void(const HttpResponsePtr&)>&& callback,
                     const std::string& thread_id) {
  auto json_body = req->getJsonObject();
  if (json_body == nullptr) {
    Json::Value ret;
    ret["message"] = "Body can't be empty";
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
    resp->setStatusCode(k400BadRequest);
    callback(resp);
    return;
  }

  auto parse_run_res = dto::RunCreateDto::FromJson(std::move(*json_body));
  if (parse_run_res.has_error()) {
    Json::Value ret;
    ret["message"] = parse_run_res.error();
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
    resp->setStatusCode(k400BadRequest);
    callback(resp);
    return;
  }

  // Create a shared state structure to hold our resources
  struct SharedState {
    dto::RunCreateDto dto;
    std::shared_ptr<drogon::ResponseStream> stream;
    explicit SharedState(dto::RunCreateDto&& d) : dto(std::move(d)) {}
  };

  // Create shared state with move-constructed DTO
  auto state = std::make_shared<SharedState>(std::move(parse_run_res.value()));

  auto sendEvents = [this, state](ResponseStreamPtr res) {
    // Store the response stream in the shared state
    state->stream = std::shared_ptr<drogon::ResponseStream>(std::move(res));

    run_srv_->CreateRunStream(
        std::move(state->dto),
        [state](const OpenAi::AssistantStreamEvent& event, bool disconnect) {
          auto parse_event_res = event.ToEvent();
          if (parse_event_res.has_value()) {
            state->stream->send(parse_event_res.value());
            if (disconnect) {
              state->stream->close();
            }
          }
        });
  };

  auto resp = HttpResponse::newAsyncStreamResponse(std::move(sendEvents));
  resp->setContentTypeString("text/event-stream");
  resp->addHeader("Cache-Control", "no-cache");
  resp->addHeader("Connection", "keep-alive");
  callback(resp);
}

void Runs::ProcessStreamRes(std::function<void(const HttpResponsePtr&)> cb,
                            std::shared_ptr<SyncQueue> q,
                            const std::string& engine_type,
                            const std::string& model_id) {
  auto err_or_done = std::make_shared<std::atomic_bool>(false);
  auto chunked_content_provider = [this, q, err_or_done, engine_type, model_id](
                                      char* buf,
                                      std::size_t buf_size) -> std::size_t {
    if (buf == nullptr) {
      LOG_TRACE << "Buf is null";
      if (!(*err_or_done)) {
        inference_srv_->StopInferencing(engine_type, model_id);
      }
      return 0;
    }

    if (*err_or_done) {
      LOG_TRACE << "Done";
      return 0;
    }

    auto [status, res] = q->wait_and_pop();

    if (status["has_error"].asBool() || status["is_done"].asBool()) {
      *err_or_done = true;
    }

    auto str = res["data"].asString();
    std::size_t n = std::min(str.size(), buf_size);
    memcpy(buf, str.data(), n);

    return n;
  };

  auto resp = cortex_utils::CreateCortexStreamResponse(chunked_content_provider,
                                                       "chat_completions.txt");
  cb(resp);
}

void Runs::RetrieveRun(const HttpRequestPtr& req,
                       std::function<void(const HttpResponsePtr&)>&& callback,
                       const std::string& thread_id,
                       const std::string& run_id) {}

void Runs::CancelRun(const HttpRequestPtr& req,
                     std::function<void(const HttpResponsePtr&)>&& callback,
                     const std::string& thread_id, const std::string& run_id) {}

void Runs::ModifyRun(const HttpRequestPtr& req,
                     std::function<void(const HttpResponsePtr&)>&& callback,
                     const std::string& thread_id, const std::string& run_id) {}

void Runs::SubmitToolOutput(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback,
    const std::string& thread_id, const std::string& run_id) {}
