#include "model_pull_cmd.h"
#include <utility>
#include "services/download_service.h"
#include "trantor/utils/Logger.h"
#include "utils/cortexso_parser.h"
#include "utils/model_callback_utils.h"

namespace commands {
ModelPullCmd::ModelPullCmd(std::string model_handle, std::string branch)
    : model_handle_(std::move(model_handle)), branch_(std::move(branch)) {}

bool ModelPullCmd::Exec() {
  auto downloadTask = cortexso_parser::getDownloadTask(model_handle_, branch_);
  if (downloadTask.has_value()) {
    DownloadService downloadService;
    downloadService.AddDownloadTask(downloadTask.value(),
                                    model_callback_utils::DownloadModelCb);
    std::cout << "Download finished" << std::endl;
    return true;
  } else {
    std::cout << "Model not found" << std::endl;
    return false;
  }
}

};  // namespace commands