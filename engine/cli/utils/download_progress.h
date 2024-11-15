#pragma once
#include <atomic>
#include <memory>
#include <string>
#include <unordered_set>
#include "common/event.h"
#include "easywsclient.hpp"

using DownloadStatus = cortex::event::DownloadEventType;
class DownloadProgress {
 public:
  bool Connect(const std::string& host, int port);

  bool Handle(const std::unordered_set<DownloadType>& event_type);

  void ForceStop() { force_stop_ = true; }

 private:
  bool should_stop() const {
    bool should_stop = true;
    for (auto const& [_, v] : status_) {
      should_stop &= (v == DownloadStatus::DownloadSuccess);
    }
    for (auto const& [_, v] : status_) {
      should_stop |= (v == DownloadStatus::DownloadError ||
                      v == DownloadStatus::DownloadStopped);
    }
    return should_stop || force_stop_;
  }

 private:
  std::unique_ptr<easywsclient::WebSocket> ws_;
  std::unordered_map<DownloadType, std::atomic<DownloadStatus>> status_;
  std::atomic<bool> force_stop_ = false;
};