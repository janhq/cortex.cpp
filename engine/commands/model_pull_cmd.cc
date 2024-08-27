#include "model_pull_cmd.h"
#include <utility>
#include "services/download_service.h"
#include "trantor/utils/Logger.h"
#include "utils/cortexso_parser.h"
#include "utils/model_callback_utils.h"

namespace commands {
ModelPullCmd::ModelPullCmd(std::string modelHandle)
    : modelHandle_(std::move(modelHandle)) {}

void ModelPullCmd::Exec() {
  auto downloadTask = cortexso_parser::getDownloadTask(modelHandle_);
  if (downloadTask.has_value()) {
    DownloadService downloadService;
    downloadService.AddDownloadTask(downloadTask.value(),
                                    model_callback_utils::DownloadModelCb);
    std::cout << "Download finished" << std::endl;
  } else {
    std::cout << "Model not found" << std::endl;
  }
}

};  // namespace commands