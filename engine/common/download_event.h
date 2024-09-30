#pragma once

#include <nlohmann/json.hpp>
#include "common/download_task.h"
#include "common/event.h"

namespace cortex::event {
using namespace nlohmann;

enum class DownloadEventType {
  DownloadStarted,
  DownloadStopped,
  DownloadUpdated,
  DownloadSuccess,
  DownloadError,
};

namespace {
std::string DownloadEventTypeToString(DownloadEventType type) {
  switch (type) {
    case DownloadEventType::DownloadStarted:
      return "DownloadStarted";
    case DownloadEventType::DownloadStopped:
      return "DownloadPaused";
    case DownloadEventType::DownloadUpdated:
      return "DownloadUpdated";
    case DownloadEventType::DownloadSuccess:
      return "DownloadSuccess";
    case DownloadEventType::DownloadError:
      return "DownloadError";
    default:
      return "Unknown";
  }
}
}  // namespace

struct DownloadEvent : public cortex::event::Event {
  std::string ToJsonString() const {
    json json{{"type", DownloadEventTypeToString(type_)},
              {"task", download_task_.ToJson()}};
    return json.dump();
  }

  DownloadEventType type_;
  DownloadTask download_task_;
};
};  // namespace cortex::event
