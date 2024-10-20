#pragma once

#include <drogon/HttpController.h>
#include <trantor/utils/Logger.h>
#include "services/model_service.h"

using namespace drogon;

class Models : public drogon::HttpController<Models, false> {
 public:
  METHOD_LIST_BEGIN
  METHOD_ADD(Models::PullModel, "/pull", Post);
  METHOD_ADD(Models::AbortPullModel, "/pull", Delete);
  METHOD_ADD(Models::ListModel, "", Get);
  METHOD_ADD(Models::GetModel, "/{1}", Get);
  METHOD_ADD(Models::UpdateModel, "/{1}", Patch);
  METHOD_ADD(Models::ImportModel, "/import", Post);
  METHOD_ADD(Models::DeleteModel, "/{1}", Delete);
  METHOD_ADD(Models::StartModel, "/start", Post);
  METHOD_ADD(Models::StopModel, "/stop", Post);
  METHOD_ADD(Models::GetModelStatus, "/status/{1}", Get);

  ADD_METHOD_TO(Models::PullModel, "/v1/models/pull", Post);
  ADD_METHOD_TO(Models::AbortPullModel, "/v1/models/pull", Delete);
  ADD_METHOD_TO(Models::ListModel, "/v1/models", Get);
  ADD_METHOD_TO(Models::GetModel, "/v1/models/{1}", Get);
  ADD_METHOD_TO(Models::UpdateModel, "/v1/models/{1}", Patch);
  ADD_METHOD_TO(Models::ImportModel, "/v1/models/import", Post);
  ADD_METHOD_TO(Models::DeleteModel, "/v1/models/{1}", Delete);
  ADD_METHOD_TO(Models::StartModel, "/v1/models/start", Post);
  ADD_METHOD_TO(Models::StopModel, "/v1/models/stop", Post);
  ADD_METHOD_TO(Models::GetModelStatus, "/v1/models/status/{1}", Get);
  METHOD_LIST_END

  explicit Models(std::shared_ptr<ModelService> model_service)
      : model_service_{model_service} {}

  void PullModel(const HttpRequestPtr& req,
                 std::function<void(const HttpResponsePtr&)>&& callback);
  void AbortPullModel(const HttpRequestPtr& req,
                      std::function<void(const HttpResponsePtr&)>&& callback);
  void ListModel(const HttpRequestPtr& req,
                 std::function<void(const HttpResponsePtr&)>&& callback) const;
  void GetModel(const HttpRequestPtr& req,
                std::function<void(const HttpResponsePtr&)>&& callback,
                const std::string& model_id) const;
  void UpdateModel(const HttpRequestPtr& req,
                   std::function<void(const HttpResponsePtr&)>&& callback,
                   const std::string& model_id) const;
  void ImportModel(
      const HttpRequestPtr& req,
      std::function<void(const HttpResponsePtr&)>&& callback) const;
  void DeleteModel(const HttpRequestPtr& req,
                   std::function<void(const HttpResponsePtr&)>&& callback,
                   const std::string& model_id);
  void SetModelAlias(
      const HttpRequestPtr& req,
      std::function<void(const HttpResponsePtr&)>&& callback) const;

  void StartModel(const HttpRequestPtr& req,
                  std::function<void(const HttpResponsePtr&)>&& callback);

  void StopModel(const HttpRequestPtr& req,
                 std::function<void(const HttpResponsePtr&)>&& callback);

  void GetModelStatus(const HttpRequestPtr& req,
                      std::function<void(const HttpResponsePtr&)>&& callback,
                      const std::string& model_id);

 private:
  std::shared_ptr<ModelService> model_service_;
};
