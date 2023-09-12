#pragma once

#include "inference_lib.h"
#include <drogon/HttpFilter.h>
using namespace drogon;

#define JSON_VALIDATOR(CLASS_NAME, JSON_SCHEMA_FILE)                           \
  class CLASS_NAME##_filter : public HttpFilter<CLASS_NAME##_filter> {         \
  public:                                                                      \
    CLASS_NAME##_filter() {                                                    \
      inference_utils::loadSchema(JSON_SCHEMA_FILE, this->schema_);            \
    }                                                                          \
    void doFilter(const HttpRequestPtr &req, FilterCallback &&fcb,             \
                  FilterChainCallback &&fccb) override;                        \
                                                                               \
  private:                                                                     \
    inference_valijson::Schema schema_;                                        \
    inference_valijson::Validator validator_;                                  \
  };                                                                           \
                                                                               \
  void CLASS_NAME##_filter::doFilter(const HttpRequestPtr &req,                \
                                     FilterCallback &&fcb,                     \
                                     FilterChainCallback &&fccb) {             \
    const auto &jsonBody = req->getJsonObject();                               \
    if (!jsonBody) {                                                           \
      Json::Value errorJson;                                                   \
      errorJson["error"] = "Malformed Json body";                              \
      auto errResp = drogon::HttpResponse::newHttpJsonResponse(errorJson);     \
      fcb(errResp);                                                            \
      return;                                                                  \
    }                                                                          \
                                                                               \
    if (!inference_utils::validate_json(this->schema_, *jsonBody,              \
                                        this->validator_)) {                   \
      Json::Value errorJson;                                                   \
      errorJson["error"] = "Please review your json body, schema not matched"; \
      auto errResp = drogon::HttpResponse::newHttpJsonResponse(errorJson);     \
      fcb(errResp);                                                            \
      return;                                                                  \
    };                                                                         \
                                                                               \
    fccb();                                                                    \
  }
