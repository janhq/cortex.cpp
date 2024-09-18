#pragma once
#include <string>
#include "httplib.h"

namespace commands {

inline bool IsServerAlive(const std::string& host, int port) {
  httplib::Client cli(host + ":" + std::to_string(port));
  auto res = cli.Get("/healthz");
  if (res && res->status == httplib::StatusCode::OK_200) {
    return true;
  }
  return false;
}

class ServerStartCmd {
 public:
  ServerStartCmd();
  bool Exec(const std::string& host, int port);
};
}  // namespace commands