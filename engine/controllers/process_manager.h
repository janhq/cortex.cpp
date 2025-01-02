#pragma once

#include <drogon/HttpController.h>
#include <drogon/HttpTypes.h>
#include "services/inference_service.h"
#include "services/model_service.h"

using namespace drogon;

class ProcessManager : public drogon::HttpController<ProcessManager, false> {
 public:
  METHOD_LIST_BEGIN
  METHOD_ADD(ProcessManager::destroy, "/destroy", Options, Delete);
  METHOD_LIST_END

  void destroy(const HttpRequestPtr& req,
               std::function<void(const HttpResponsePtr&)>&& callback);

  ProcessManager(std::shared_ptr<InferenceService> inference_service,
                 std::shared_ptr<ModelService> model_service)
      : inference_service_(inference_service), model_service_(model_service) {}

 private:
  std::shared_ptr<InferenceService> inference_service_;
  std::shared_ptr<ModelService> model_service_;
};
