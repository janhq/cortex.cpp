#pragma once

#include <drogon/HttpController.h>
#include <drogon/HttpTypes.h>
#include "services/download_service.h"

using namespace drogon;

class ProcessManager : public drogon::HttpController<ProcessManager, false> {
 public:
  METHOD_LIST_BEGIN
  METHOD_ADD(ProcessManager::destroy, "/destroy", Delete);
  METHOD_LIST_END

  explicit ProcessManager(std::shared_ptr<DownloadService> download_service)
      : download_service_{download_service} {}

  void destroy(const HttpRequestPtr& req,
               std::function<void(const HttpResponsePtr&)>&& callback);

 private:
  std::shared_ptr<DownloadService> download_service_;
};
