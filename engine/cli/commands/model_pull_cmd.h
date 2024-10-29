#pragma once

#include "services/model_service.h"

namespace commands {

class ModelPullCmd {
 public:
  explicit ModelPullCmd(std::shared_ptr<DownloadService> download_service)
      : model_service_{ModelService(download_service)} {};
  explicit ModelPullCmd(const ModelService& model_service)
      : model_service_{model_service} {};
  std::optional<std::string> Exec(const std::string& host, int port,
                                  const std::string& input);

 private:
  bool AbortModelPull(const std::string& host, int port,
                      const std::string& task_id);

 private:
  ModelService model_service_;
};
}  // namespace commands
