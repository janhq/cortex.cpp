#include "chat_cmd.h"
#include "httplib.h"

#include "database/models.h"
#include "model_status_cmd.h"
#include "server_start_cmd.h"
#include "trantor/utils/Logger.h"
#include "utils/logging_utils.h"
#include "run_cmd.h"

namespace commands {
void ChatCmd::Exec(const std::string& host, int port,
                   const std::string& model_handle) {
  RunCmd rc(host, port, model_handle);
  rc.Exec(true /*chat_flag*/);
}
};  // namespace commands