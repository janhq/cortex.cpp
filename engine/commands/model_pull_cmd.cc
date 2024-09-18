#include "model_pull_cmd.h"
#include "utils/cortexso_parser.h"

namespace commands {
void ModelPullCmd::Exec(const std::string& input) {
  model_service_.DownloadModel(input);
}
};  // namespace commands
