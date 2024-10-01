#include "engine_uninstall_cmd.h"
#include "services/engine_service.h"
#include "utils/logging_utils.h"

namespace commands {

void EngineUninstallCmd::Exec(const std::string& engine) {
  auto result = engine_service_.UninstallEngine(engine);

  if (result.has_error()) {
    CLI_LOG(result.error());
  } else {
    CLI_LOG("Engine " + engine + " uninstalled successfully!");
  }
}
};  // namespace commands
