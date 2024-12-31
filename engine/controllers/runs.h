#pragma once

#include <drogon/HttpController.h>
#include <trantor/utils/Logger.h>
#include "services/inference_service.h"
#include "services/run_service.h"

using namespace drogon;

class Runs : public drogon::HttpController<Runs, false> {
 public:
  METHOD_LIST_BEGIN

  ADD_METHOD_TO(Runs::CreateRun, "/v1/threads/{thread_id}/runs", Options, Post);

  ADD_METHOD_TO(Runs::ModifyRun, "/v1/threads/{thread_id}/runs/{run_id}",
                Options, Post);

  ADD_METHOD_TO(Runs::CancelRun, "/v1/threads/{thread_id}/runs/{run_id}/cancel",
                Options, Post);

  ADD_METHOD_TO(Runs::RetrieveRun, "/v1/threads/{thread_id}/runs/{run_id}",
                Get);

  ADD_METHOD_TO(Runs::ListRuns, "/v1/threads/{thread_id}/runs", Get);

  ADD_METHOD_TO(Runs::SubmitToolOutput,
                "/v1/threads/{thread_id}/runs/{run_id}/submit_tool_outputs",
                Options, Post);
  METHOD_LIST_END

  explicit Runs(std::shared_ptr<RunService> run_service,
                std::shared_ptr<InferenceService> inference_service)
      : run_srv_{run_service}, inference_srv_{inference_service} {}

  void CreateRun(const HttpRequestPtr& req,
                 std::function<void(const HttpResponsePtr&)>&& callback,
                 const std::string& thread_id);

  void RetrieveRun(const HttpRequestPtr& req,
                   std::function<void(const HttpResponsePtr&)>&& callback,
                   const std::string& thread_id, const std::string& run_id);

  void CancelRun(const HttpRequestPtr& req,
                 std::function<void(const HttpResponsePtr&)>&& callback,
                 const std::string& thread_id, const std::string& run_id);

  void ModifyRun(const HttpRequestPtr& req,
                 std::function<void(const HttpResponsePtr&)>&& callback,
                 const std::string& thread_id, const std::string& run_id);

  void SubmitToolOutput(const HttpRequestPtr& req,
                        std::function<void(const HttpResponsePtr&)>&& callback,
                        const std::string& thread_id,
                        const std::string& run_id);

  void ListRuns(const HttpRequestPtr& req,
                std::function<void(const HttpResponsePtr&)>&& callback,
                const std::string& thread_id, std::optional<std::string> limit,
                std::optional<std::string> order,
                std::optional<std::string> after,
                std::optional<std::string> before) const;

 private:
  std::shared_ptr<RunService> run_srv_;
  std::shared_ptr<InferenceService> inference_srv_;

  void ProcessStreamRes(std::function<void(const HttpResponsePtr&)> cb,
                        std::shared_ptr<SyncQueue> q,
                        const std::string& engine_type,
                        const std::string& model_id);
};
