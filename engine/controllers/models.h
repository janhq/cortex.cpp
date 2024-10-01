#pragma once

#include <drogon/HttpController.h>
#include <trantor/utils/Logger.h>
#include "services/model_service.h"

using namespace drogon;

class Models : public drogon::HttpController<Models> {
 public:
  explicit Models() : model_service_{ModelService()} {};

  METHOD_LIST_BEGIN
  METHOD_ADD(Models::PullModel, "/pull", Post);
  METHOD_ADD(Models::ListModel, "", Get);
  METHOD_ADD(Models::GetModel, "/get", Post);
  METHOD_ADD(Models::UpdateModel, "/update/", Post);
  METHOD_ADD(Models::ImportModel, "/import", Post);
  METHOD_ADD(Models::DeleteModel, "/{1}", Delete);
  METHOD_ADD(Models::SetModelAlias, "/alias", Post);
  METHOD_ADD(Models::StartModel, "/start", Post);
  METHOD_ADD(Models::StopModel, "/stop", Post);
  METHOD_LIST_END

  void PullModel(const HttpRequestPtr& req,
                 std::function<void(const HttpResponsePtr&)>&& callback);
  void ListModel(const HttpRequestPtr& req,
                 std::function<void(const HttpResponsePtr&)>&& callback) const;
  void GetModel(const HttpRequestPtr& req,
                std::function<void(const HttpResponsePtr&)>&& callback) const;
  void UpdateModel(
      const HttpRequestPtr& req,
      std::function<void(const HttpResponsePtr&)>&& callback) const;
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

 private:
  ModelService model_service_;
};
