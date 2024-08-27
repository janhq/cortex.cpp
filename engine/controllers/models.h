#pragma once

#include <drogon/HttpController.h>
#include <trantor/utils/Logger.h>
#include "services/download_service.h"
#include "utils/cortex_utils.h"
#include "utils/cortexso_parser.h"
#include "utils/http_util.h"

using namespace drogon;

class Models : public drogon::HttpController<Models> {
 public:
  METHOD_LIST_BEGIN
  METHOD_ADD(Models::PullModel, "/pull", Post);
  METHOD_ADD(Models::ListModel, "/list", Get);
  METHOD_LIST_END

  void PullModel(const HttpRequestPtr& req,
                 std::function<void(const HttpResponsePtr&)>&& callback) const;
  void ListModel(const HttpRequestPtr& req,
                 std::function<void(const HttpResponsePtr&)>&& callback) const;
};