#pragma once

#include <drogon/HttpController.h>
#include <drogon/HttpRequest.h>
#include <trantor/utils/Logger.h>
#include "utils/cortexso_parser.h"

using namespace drogon;

class Engines : public drogon::HttpController<Engines> {
 public:
  METHOD_LIST_BEGIN
  METHOD_ADD(Engines::InstallEngine, "/{1}/init", Post);
  METHOD_ADD(Engines::UninstallEngine, "/{1}", Delete);
  METHOD_ADD(Engines::ListEngine, "", Get);
  METHOD_ADD(Engines::GetEngine, "/{1}", Get);
  METHOD_LIST_END

  void InstallEngine(const HttpRequestPtr& req,
                     std::function<void(const HttpResponsePtr&)>&& callback,
                     const std::string& engine) const;

  void ListEngine(const HttpRequestPtr& req,
                  std::function<void(const HttpResponsePtr&)>&& callback) const;

  void GetEngine(const HttpRequestPtr& req,
                 std::function<void(const HttpResponsePtr&)>&& callback,
                 const std::string& engine) const;

  void UninstallEngine(const HttpRequestPtr& req,
                       std::function<void(const HttpResponsePtr&)>&& callback,
                       const std::string& engine) const;
};
