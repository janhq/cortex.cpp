#include "download_progress.h"
#include <chrono>
#include "common/event.h"
#include "indicators.hpp"
#include "utils/format_utils.h"
#include "utils/json_helper.h"
#include "utils/logging_utils.h"

bool DownloadProgress::Connect(const std::string& host, int port) {
  if (ws_) {
    CTL_INF("Already connected!");
    return true;
  }
  ws_.reset(easywsclient::WebSocket::from_url(
      "ws://" + host + ":" + std::to_string(port) + "/events"));
  if (!!ws_)
    return false;

  return true;
}

bool DownloadProgress::Handle(const std::string& id) {
  assert(!!ws_);
  status_ = DownloadStatus::DownloadStarted;
  std::unique_ptr<indicators::DynamicProgress<indicators::ProgressBar>> bars;

  std::vector<std::unique_ptr<indicators::ProgressBar>> items;
  indicators::show_console_cursor(false);
  auto handle_message = [this, &bars, &items, id](const std::string& message) {
    CTL_INF(message);

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
    // Ignore other task ids
    if (ev.download_task_.id != id) {
      return;
    }

    status_ = ev.type_;

    if (!bars) {
      bars = std::make_unique<
          indicators::DynamicProgress<indicators::ProgressBar>>();
      for (auto& i : ev.download_task_.items) {
        items.emplace_back(std::make_unique<indicators::ProgressBar>(
            indicators::option::BarWidth{50}, indicators::option::Start{"["},
            indicators::option::Fill{"="}, indicators::option::Lead{">"},
            indicators::option::End{"]"},
            indicators::option::PrefixText{pad_string(i.id)},
            indicators::option::ForegroundColor{indicators::Color::white},
            indicators::option::ShowRemainingTime{true}));
        bars->push_back(*(items.back()));
      }
    } else {
      for (int i = 0; i < ev.download_task_.items.size(); i++) {
        auto& it = ev.download_task_.items[i];
        uint64_t downloaded = it.downloadedBytes.value_or(0);
        uint64_t total = it.bytes.value_or(9999);
        if (status_ == DownloadStatus::DownloadUpdated) {
          (*bars)[i].set_option(indicators::option::PrefixText{
              pad_string(it.id) +
              std::to_string(
                  int(static_cast<double>(downloaded) / total * 100)) +
              '%'});
          (*bars)[i].set_progress(
              int(static_cast<double>(downloaded) / total * 100));
          (*bars)[i].set_option(indicators::option::PostfixText{
              format_utils::BytesToHumanReadable(downloaded) + "/" +
              format_utils::BytesToHumanReadable(total)});
        } else if (status_ == DownloadStatus::DownloadSuccess) {
          (*bars)[i].set_option(
              indicators::option::PrefixText{pad_string(it.id) + "100%"});
          (*bars)[i].set_progress(100);
          auto total_str = format_utils::BytesToHumanReadable(total);
          (*bars)[i].set_option(
              indicators::option::PostfixText{total_str + "/" + total_str});

          CTL_INF("Download success");
        }
      }
    }
  };

  while (ws_->getReadyState() != easywsclient::WebSocket::CLOSED &&
         !should_stop()) {
    ws_->poll();
    ws_->dispatch(handle_message);
  }
  indicators::show_console_cursor(true);
  if (status_ == DownloadStatus::DownloadError)
    return false;
  return true;
}