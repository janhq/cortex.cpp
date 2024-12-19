#pragma once

#include <drogon/HttpController.h>
#include <trantor/utils/Logger.h>
#include "services/assistant_service.h"

using namespace drogon;

class Assistants : public drogon::HttpController<Assistants, false> {
 public:
  METHOD_LIST_BEGIN
  ADD_METHOD_TO(Assistants::RetrieveAssistant, "/v1/assistants/{assistant_id}",
                Get);

  ADD_METHOD_TO(Assistants::CreateAssistant, "/v1/assistants/{assistant_id}",
                Options, Post);

  ADD_METHOD_TO(Assistants::ModifyAssistant, "/v1/assistants/{assistant_id}",
                Options, Patch);
  METHOD_LIST_END

  explicit Assistants(std::shared_ptr<AssistantService> assistant_srv)
      : assistant_service_{assistant_srv} {};

  void RetrieveAssistant(const HttpRequestPtr& req,
                         std::function<void(const HttpResponsePtr&)>&& callback,
                         const std::string& assistant_id) const;

  void CreateAssistant(const HttpRequestPtr& req,
                       std::function<void(const HttpResponsePtr&)>&& callback,
                       const std::string& assistant_id);

  void ModifyAssistant(const HttpRequestPtr& req,
                       std::function<void(const HttpResponsePtr&)>&& callback,
                       const std::string& assistant_id);

 private:
  std::shared_ptr<AssistantService> assistant_service_;
};
