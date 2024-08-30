#include "stop_server_cmd.h"
#include "httplib.h"
#include "trantor/utils/Logger.h"

namespace commands {
StopServerCmd::StopServerCmd(std::string host, int port)
    : host_(std::move(host)), port_(port) {}

void StopServerCmd::Exec() {
  httplib::Client cli(host_ + ":" + std::to_string(port_));
  auto res = cli.Delete("/processManager/destroy");
  if (res) {
    LOG_INFO << res->body;
  } else {
    auto err = res.error();
    LOG_WARN << "HTTP error: " << httplib::to_string(err);
  }
}

};  // namespace commands