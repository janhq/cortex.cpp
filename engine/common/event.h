#pragma once

#include <eventpp/eventqueue.h>
#include <nlohmann/json.hpp>
#include <string>
#include "common/download_task.h"
#include "eventpp/utilities/anydata.h"

namespace cortex::event {
using namespace nlohmann;

enum class EventType {
  DownloadEvent,
  ExitEvent,
};

struct Event {};

struct ExitEvent : public cortex::event::Event {
  std::string message;
};

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
}  // namespace cortex::event

constexpr std::size_t eventMaxSize =
    eventpp::maxSizeOf<cortex::event::Event, cortex::event::DownloadEvent,
                       cortex::event::ExitEvent, std::string>();
