#include "model_del_cmd.h"
#include "utils/logging_utils.h"

namespace commands {
void ModelDelCmd::Exec(const std::string& model_handle) {
  auto result = model_service_.DeleteModel(model_handle);
  if (result.has_error()) {
    CLI_LOG(result.error());
  } else {
    CLI_LOG("Model " + model_handle + " deleted successfully");
  }
}
}  // namespace commands
