#include "server_stop_cmd.h"
#include "httplib.h"
#include "utils/logging_utils.h"

namespace commands {
ServerStopCmd::ServerStopCmd(std::string host, int port)
    : host_(std::move(host)), port_(port) {}

void ServerStopCmd::Exec() {
  httplib::Client cli(host_ + ":" + std::to_string(port_));
  auto res = cli.Delete("/processManager/destroy");
  if (res) {
    CLI_LOG("Server stopped!");
  } else {
    auto err = res.error();
    CTL_ERR("HTTP error: " << httplib::to_string(err));
  }
}

};  // namespace commands
