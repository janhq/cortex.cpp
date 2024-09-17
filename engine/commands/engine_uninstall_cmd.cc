#include "engine_uninstall_cmd.h"
#include "services/engine_service.h"
#include "utils/logging_utils.h"

namespace commands {

void EngineUninstallCmd::Exec(const std::string& engine) {
  engine_service_.UninstallEngine(engine);
  CLI_LOG("Engine " << engine << " uninstalled successfully!");
}
};  // namespace commands
