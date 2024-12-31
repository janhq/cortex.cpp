#pragma once

#include <drogon/HttpController.h>
#include <trantor/utils/Logger.h>
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

  ADD_METHOD_TO(Runs::SubmitToolOutput,
                "/v1/threads/{thread_id}/runs/{run_id}/submit_tool_outputs",
                Options, Post);
  METHOD_LIST_END

  explicit Runs(std::shared_ptr<RunService> run_service)
      : run_service_{run_service} {}

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

 private:
  std::shared_ptr<RunService> run_service_;
};
