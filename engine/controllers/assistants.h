#pragma once

#include <drogon/HttpController.h>
#include <trantor/utils/Logger.h>
#include "services/assistant_service.h"

using namespace drogon;

class Assistants : public drogon::HttpController<Assistants, false> {
  constexpr static auto kOpenAiAssistantKeyV2 = "openai-beta";
  constexpr static auto kOpenAiAssistantValueV2 = "assistants=v2";

 public:
  METHOD_LIST_BEGIN
  ADD_METHOD_TO(
      Assistants::ListAssistants,
      "/v1/"
      "assistants?limit={limit}&order={order}&after={after}&before={before}",
      Get);

  ADD_METHOD_TO(Assistants::DeleteAssistant, "/v1/assistants/{assistant_id}",
                Options, Delete);

  ADD_METHOD_TO(Assistants::RetrieveAssistant, "/v1/assistants/{assistant_id}",
                Get);

  ADD_METHOD_TO(Assistants::CreateAssistant, "/v1/assistants/{assistant_id}",
                Options, Post);

  ADD_METHOD_TO(Assistants::CreateAssistantV2, "/v1/assistants", Options, Post);

  ADD_METHOD_TO(Assistants::ModifyAssistant, "/v1/assistants/{assistant_id}",
                Options, Patch);

  METHOD_LIST_END

  explicit Assistants(std::shared_ptr<AssistantService> assistant_srv)
      : assistant_service_{assistant_srv} {};

  void ListAssistants(const HttpRequestPtr& req,
                      std::function<void(const HttpResponsePtr&)>&& callback,
                      std::optional<std::string> limit,
                      std::optional<std::string> order,
                      std::optional<std::string> after,
                      std::optional<std::string> before) const;

  void RetrieveAssistant(const HttpRequestPtr& req,
                         std::function<void(const HttpResponsePtr&)>&& callback,
                         const std::string& assistant_id) const;

  void RetrieveAssistantV2(
      const HttpRequestPtr& req,
      std::function<void(const HttpResponsePtr&)>&& callback,
      const std::string& assistant_id) const;

  void DeleteAssistant(const HttpRequestPtr& req,
                       std::function<void(const HttpResponsePtr&)>&& callback,
                       const std::string& assistant_id);

  void CreateAssistant(const HttpRequestPtr& req,
                       std::function<void(const HttpResponsePtr&)>&& callback,
                       const std::string& assistant_id);

  void CreateAssistantV2(
      const HttpRequestPtr& req,
      std::function<void(const HttpResponsePtr&)>&& callback);

  void ModifyAssistant(const HttpRequestPtr& req,
                       std::function<void(const HttpResponsePtr&)>&& callback,
                       const std::string& assistant_id);

  void ModifyAssistantV2(const HttpRequestPtr& req,
                         std::function<void(const HttpResponsePtr&)>&& callback,
                         const std::string& assistant_id);

 private:
  std::shared_ptr<AssistantService> assistant_service_;
};
