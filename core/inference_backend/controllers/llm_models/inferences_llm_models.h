#pragma once

#include "validation_macro.h"
#include <drogon/HttpController.h>
#include <inference_lib.h>

using namespace drogon;

JSON_VALIDATOR(llm_models, "/workspace/workdir/inference_backend/schemas/"
                           "openai_compatible_schema.json");

namespace inferences {
class llm_models : public drogon::HttpController<llm_models> {
public:
  METHOD_LIST_BEGIN
  METHOD_ADD(llm_models::chatCompletion, "chat_completion", Post,
             "llm_models_filter");
  METHOD_LIST_END
  void chatCompletion(const HttpRequestPtr &req,
                      std::function<void(const HttpResponsePtr &)> &&callback);

private:
  int ids_count;
};
} // namespace inferences