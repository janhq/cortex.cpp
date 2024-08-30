#pragma once

#include <drogon/HttpController.h>

using namespace drogon;

class prelight : public drogon::HttpController<prelight> {
public:
  METHOD_LIST_BEGIN
  ADD_METHOD_TO(prelight::handlePrelight, "/v1/chat/completions", Options);
  ADD_METHOD_TO(prelight::handlePrelight, "/v1/embeddings", Options);
  ADD_METHOD_TO(prelight::handlePrelight, "/v1/audio/transcriptions", Options);
  ADD_METHOD_TO(prelight::handlePrelight, "/v1/audio/translations", Options);
  METHOD_LIST_END

  void handlePrelight(const HttpRequestPtr &req,
                      std::function<void(const HttpResponsePtr &)> &&callback);
};
