#include "engine_install_cmd.h"
#include "utils/logging_utils.h"

namespace commands {

void EngineInstallCmd::Exec(const std::string& engine,
                            const std::string& version, const std::string& src,
                            bool cpu_only) {
  auto result = engine_service_.InstallEngine(engine, version, src, cpu_only);
  if (result.has_error()) {
    CLI_LOG(result.error());
  } else if (result && result.value()) {
    CLI_LOG("Engine " << engine << " installed successfully!");
  }
}
};  // namespace commands
