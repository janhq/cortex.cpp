#include "model_pull_cmd.h"
#include <utility>
#include "services/download_service.h"
#include "utils/cortexso_parser.h"
#include "utils/logging_utils.h"
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
    CTL_INF("Download finished");
    return true;
  } else {
    CTL_ERR("Model not found");
    return false;
  }
}

};  // namespace commands
