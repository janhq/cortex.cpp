#include "engine_install_cmd.h"
// clang-format off
#include "utils/cortexso_parser.h" 
#include "utils/archive_utils.h"
// clang-format on
#include "utils/cuda_toolkit_utils.h"
#include "utils/engine_matcher_utils.h"
#include "utils/logging_utils.h"

namespace commands {

void EngineInstallCmd::Exec(const std::string& engine,
                            const std::string& version) {
  engine_service_.InstallEngine(engine, version);
  CLI_LOG("Engine " << engine << " installed successfully!");
}
};  // namespace commands
