#include "engine_uninstall_cmd.h"
#include "services/engine_service.h"
#include "utils/logging_utils.h"

namespace commands {

EngineUninstallCmd::EngineUninstallCmd(std::string engine)
    : engine_{std::move(engine)} {}

void EngineUninstallCmd::Exec() const {
  CTL_INF("Uninstall engine " + engine_);
  auto engine_service = EngineService();

  try {
    engine_service.UninstallEngine(engine_);
    CLI_LOG("Engine " << engine_ << " uninstalled successfully!")
  } catch (const std::exception& e) {
    CLI_LOG("Failed to uninstall engine " << engine_ << ": " << e.what());
  }
}
};  // namespace commands
