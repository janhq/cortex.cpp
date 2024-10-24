#pragma once

#include <eventpp/eventqueue.h>
#include <string>
#include "common/download_task.h"
#include "eventpp/utilities/anydata.h"
#include "utils/json_helper.h"

namespace cortex::event {

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
      return "DownloadStopped";
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

inline DownloadEventType DownloadEventTypeFromString(const std::string& str) {
  if (str == "DownloadStarted") {
    return DownloadEventType::DownloadStarted;
  } else if (str == "DownloadStopped") {
    return DownloadEventType::DownloadStopped;
  } else if (str == "DownloadUpdated") {
    return DownloadEventType::DownloadUpdated;
  } else if (str == "DownloadSuccess") {
    return DownloadEventType::DownloadSuccess;
  } else if (str == "DownloadError") {
    return DownloadEventType::DownloadError;
  } else {
    return DownloadEventType::DownloadError;
  }
}
}  // namespace

struct DownloadEvent : public cortex::event::Event {
  DownloadEventType type_;
  DownloadTask download_task_;

  std::string ToJsonString() const {
    Json::Value root;
    root["type"] = DownloadEventTypeToString(type_);
    root["task"] = download_task_.ToJsonCpp();
    return json_helper::DumpJsonString(root);
  }
};

inline DownloadEvent GetDownloadEventFromJson(const Json::Value& item_json) {
  DownloadEvent ev;
  if (!item_json["type"].isNull()) {
    ev.type_ = DownloadEventTypeFromString(item_json["type"].asString());
  }

  if (!item_json["task"].isNull()) {
    ev.download_task_ = common::GetDownloadTaskFromJson(item_json["task"]);
  }
  return ev;
}
};  // namespace cortex::event

constexpr std::size_t eventMaxSize =
    eventpp::maxSizeOf<cortex::event::Event, cortex::event::DownloadEvent,
                       cortex::event::ExitEvent, std::string>();
