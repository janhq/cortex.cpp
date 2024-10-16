#pragma once

#include <string>
#include "services/download_service.h"

namespace commands {
class ChatCmd {
 public:
  void Exec(const std::string& host, int port, const std::string& model_handle,
            std::shared_ptr<DownloadService> download_service);
};
}  // namespace commands
