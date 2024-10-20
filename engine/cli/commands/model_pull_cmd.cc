#include "model_pull_cmd.h"
#include "utils/logging_utils.h"

namespace commands {
void ModelPullCmd::Exec(const std::string& input) {
  auto result = model_service_.DownloadModel(input);
  if (result.has_error()) {
    CLI_LOG(result.error());
  }
}
};  // namespace commands
