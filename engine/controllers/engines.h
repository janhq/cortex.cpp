#pragma once

#include <drogon/HttpController.h>
#include <trantor/utils/Logger.h>
#include "services/download_service.h"
#include "utils/cortex_utils.h"
#include "utils/cortexso_parser.h"
#include "utils/http_util.h"

using namespace drogon;

class Engines : public drogon::HttpController<Engines> {
 public:
  METHOD_LIST_BEGIN
  METHOD_ADD(Engines::InitEngine, "/{1}/init", Post);
  METHOD_LIST_END

  void InitEngine(const HttpRequestPtr& req,
                  std::function<void(const HttpResponsePtr&)>&& callback,
                  const std::string& engine) const;
};
