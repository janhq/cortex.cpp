#include "download_progress.h"
#include <chrono>
#include <limits>
#include "common/event.h"
#include "indicators/dynamic_progress.hpp"
#include "indicators/progress_bar.hpp"
#include "utils/engine_constants.h"
#include "utils/format_utils.h"
#include "utils/json_helper.h"
#include "utils/logging_utils.h"

namespace {
std::string Repo2Engine(const std::string& r) {
  if (r == kLlamaRepo) {
    return kLlamaEngine;
  } else if (r == kOnnxRepo) {
    return kOnnxEngine;
  } else if (r == kTrtLlmRepo) {
    return kTrtLlmEngine;
  }
  return r;
};

std::string TimeDownloadFormat(int seconds) {
  // Constants for time units
  const uint64_t kSecondsInMinute = 60;
  const uint64_t kSecondsInHour = kSecondsInMinute * 60;
  const uint64_t kSecondsInDay = kSecondsInHour * 24;

  uint64_t days = seconds / kSecondsInDay;
  seconds %= kSecondsInDay;

  uint64_t hours = seconds / kSecondsInHour;
  seconds %= kSecondsInHour;

  uint64_t minutes = seconds / kSecondsInMinute;
  seconds %= kSecondsInMinute;

  std::ostringstream oss;

  auto pad = [](const std::string& v) -> std::string {
    if(v.size() == 1) return "0" + v;
    return v;
  };

  if (days > 0) {
    oss << days << "d:";
  }
  if (hours > 0 || days > 0) {
    oss << hours << "h:";
  }
  oss << pad(std::to_string(minutes)) << "m:";

  oss << pad(std::to_string(seconds)) << "s";

  return oss.str();
};
}  // namespace
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

bool DownloadProgress::Handle(const DownloadType& event_type) {
  assert(!!ws_);
#if defined(_WIN32)
  HANDLE h_out = GetStdHandle(STD_OUTPUT_HANDLE);
  DWORD dw_original_out_mode = 0;
  if (h_out != INVALID_HANDLE_VALUE) {
    GetConsoleMode(h_out, &dw_original_out_mode);

    // Enable ANSI escape code processing
    DWORD dw_requested_out_mode =
        dw_original_out_mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    if (!SetConsoleMode(h_out, dw_requested_out_mode)) {
      SetConsoleMode(h_out, dw_original_out_mode);
    }
  }
#endif
  status_ = DownloadStatus::DownloadStarted;
  std::unique_ptr<indicators::DynamicProgress<indicators::ProgressBar>> bars;

  std::vector<std::unique_ptr<indicators::ProgressBar>> items;
  indicators::show_console_cursor(false);
  auto start = std::chrono::steady_clock::now();
  auto handle_message = [this, &bars, &items, event_type,
                         start](const std::string& message) {
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
    // Ignore other task type
    if (ev.download_task_.type != event_type) {
      return;
    }
    auto now = std::chrono::steady_clock::now();
    if (!bars) {
      bars = std::make_unique<
          indicators::DynamicProgress<indicators::ProgressBar>>();
      for (auto& i : ev.download_task_.items) {
        items.emplace_back(std::make_unique<indicators::ProgressBar>(
            indicators::option::BarWidth{50}, indicators::option::Start{"["},
            indicators::option::Fill{"="}, indicators::option::Lead{">"},
            indicators::option::End{"]"},
            indicators::option::PrefixText{pad_string(Repo2Engine(i.id))},
            indicators::option::ForegroundColor{indicators::Color::white},
            indicators::option::ShowRemainingTime{false}));
        bars->push_back(*(items.back()));
      }
    }
    for (int i = 0; i < ev.download_task_.items.size(); i++) {
      auto& it = ev.download_task_.items[i];
      if (ev.type_ == DownloadStatus::DownloadUpdated) {
        uint64_t downloaded = it.downloadedBytes.value_or(0u);
        uint64_t total =
            it.bytes.value_or(std::numeric_limits<uint64_t>::max());
        auto d = std::chrono::duration_cast<std::chrono::seconds>(now - start)
                     .count();
        uint64_t bytes_per_sec = downloaded / (d + 1);
        std::string time_remaining;
        if(downloaded == total || bytes_per_sec == 0) {
          time_remaining = "00m:00s";
        } else {
          time_remaining = TimeDownloadFormat((total - downloaded) / bytes_per_sec);
        }
        
        (*bars)[i].set_option(indicators::option::PrefixText{
            pad_string(Repo2Engine(it.id)) +
            std::to_string(int(static_cast<double>(downloaded) / total * 100)) +
            '%'});
        (*bars)[i].set_progress(
            int(static_cast<double>(downloaded) / total * 100));
        (*bars)[i].set_option(indicators::option::PostfixText{
            time_remaining + " " +
            format_utils::BytesToHumanReadable(downloaded) + "/" +
            format_utils::BytesToHumanReadable(total)});
      } else if (ev.type_ == DownloadStatus::DownloadSuccess) {
        uint64_t total =
            it.bytes.value_or(std::numeric_limits<uint64_t>::max());
        (*bars)[i].set_progress(100);
        auto total_str = format_utils::BytesToHumanReadable(total);
        (*bars)[i].set_option(indicators::option::PostfixText{
            "00m:00s " + total_str + "/" + total_str});
        (*bars)[i].set_option(indicators::option::PrefixText{
            pad_string(Repo2Engine(it.id)) + "100%"});
        (*bars)[i].set_progress(100);

        CTL_INF("Download success");
      }
    }

    status_ = ev.type_;
  };

  while (ws_->getReadyState() != easywsclient::WebSocket::CLOSED &&
         !should_stop()) {
    ws_->poll();
    ws_->dispatch(handle_message);
  }
  indicators::show_console_cursor(true);
#if defined(_WIN32)
  if (dw_original_out_mode != 0 && h_out != INVALID_HANDLE_VALUE) {
    SetConsoleMode(h_out, dw_original_out_mode);
  }
#endif
  if (status_ == DownloadStatus::DownloadError)
    return false;
  return true;
}
