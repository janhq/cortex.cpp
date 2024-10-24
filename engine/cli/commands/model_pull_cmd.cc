#include "model_pull_cmd.h"
#include <memory>
#include "cli/utils/easywsclient.hpp"
#include "cli/utils/indicators.hpp"
#include "common/event.h"
#include "server_start_cmd.h"
#include "utils/format_utils.h"
#include "utils/json_helper.h"
#include "utils/logging_utils.h"

namespace commands {
void ModelPullCmd::Exec(const std::string& host, int port,
                        const std::string& input) {
  // Start server if server is not started yet
  if (!commands::IsServerAlive(host, port)) {
    CLI_LOG("Starting server ...");
    commands::ServerStartCmd ssc;
    if (!ssc.Exec(host, port)) {
      return;
    }
  }

  httplib::Client cli(host + ":" + std::to_string(port));
  Json::Value json_data;
  json_data["model"] = input;
  auto data_str = json_data.toStyledString();
  cli.set_read_timeout(std::chrono::seconds(60));
  auto res = cli.Post("/v1/models/pull", httplib::Headers(), data_str.data(),
                      data_str.size(), "application/json");

  if (res) {
    if (res->status == httplib::StatusCode::OK_200) {
      // CLI_LOG("OK");
    } else {
      CTL_ERR("Error:");
      return;
    }
  } else {
    auto err = res.error();
    CTL_ERR("HTTP error: " << httplib::to_string(err));
    return;
  }

  std::unique_ptr<indicators::DynamicProgress<indicators::BlockProgressBar>>
      bars;

  std::vector<std::unique_ptr<indicators::BlockProgressBar>> items;

  auto handle_message = [&bars, &items](const std::string& message) {
    // std::cout << message << std::endl;

    auto pad_string = [](const std::string& str,
                         size_t max_length = 20) -> std::string {
      // Check the length of the input string
      if (str.length() >= max_length) {
        return str.substr(
            0, max_length);  // Return truncated string if it's too long
      }

      // Calculate the number of spaces needed
      size_t padding_size = max_length - str.length();

      // Create a new string with the original string followed by spaces
      return str + std::string(padding_size, ' ');
    };

    auto ev = cortex::event::GetDownloadEventFromJson(
        json_helper::ParseJsonString(message));
    // std::cout << downloaded << " " << total << std::endl;
    if (!bars) {
      bars = std::make_unique<
          indicators::DynamicProgress<indicators::BlockProgressBar>>();
      for (auto& i : ev.download_task_.items) {
        items.emplace_back(std::make_unique<indicators::BlockProgressBar>(
            indicators::option::BarWidth{50}, indicators::option::Start{"|"},
            // indicators::option::Fill{"■"}, indicators::option::Lead{"■"},
            // indicators::option::Remainder{" "},
            indicators::option::End{"|"}, indicators::option::PrefixText{pad_string(i.id)},
            indicators::option::PostfixText{"Downloading files"},
            indicators::option::ForegroundColor{indicators::Color::white},
            indicators::option::ShowRemainingTime{true},
            indicators::option::FontStyles{std::vector<indicators::FontStyle>{
                indicators::FontStyle::bold}}));
        bars->push_back(*(items.back()));
      }
    } else {
      for (int i = 0; i < ev.download_task_.items.size(); i++) {
        auto& it = ev.download_task_.items[i];
        uint64_t downloaded = it.downloadedBytes.value_or(0);
        uint64_t total = it.bytes.value_or(9999);
        (*bars)[i].set_progress(static_cast<double>(downloaded) / total * 100);
        (*bars)[i].set_option(indicators::option::PostfixText{
            format_utils::BytesToHumanReadable(downloaded) + "/" +
            format_utils::BytesToHumanReadable(total)});
      }
    }
  };

  auto ws = easywsclient::WebSocket::from_url("ws://" + host + ":" +
                                              std::to_string(port) + "/events");
  // auto result = model_service_.DownloadModel(input);
  // if (result.has_error()) {
  //   CLI_LOG(result.error());
  // }
  while (ws->getReadyState() != easywsclient::WebSocket::CLOSED) {
    ws->poll();
    ws->dispatch(handle_message);
  }
  std::cout << "Done" << std::endl;
  delete ws;
}
};  // namespace commands
