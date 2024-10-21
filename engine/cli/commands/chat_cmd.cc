#include "chat_cmd.h"
#include "run_cmd.h"

namespace commands {
void ChatCmd::Exec(const std::string& host, int port,
                   const std::string& model_handle,
                   std::shared_ptr<DownloadService> download_service) {
  RunCmd rc(host, port, model_handle, download_service);
  rc.Exec(false /*detach mode*/);
}
};  // namespace commands
