#pragma once

#include <drogon/HttpController.h>

using namespace drogon;

class llm_models : public drogon::HttpController<llm_models> {
public:
  METHOD_LIST_BEGIN
  METHOD_ADD(llm_models::chatCompletion, "chat_completion", Post);
  METHOD_LIST_END
  void chatCompletion(const HttpRequestPtr &req,
                      std::function<void(const HttpResponsePtr &)> &&callback);

private:
  int ids_count;
};