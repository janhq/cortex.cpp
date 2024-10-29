#pragma once
#include <atomic>
#include <memory>
#include <string>
#include "common/event.h"
#include "easywsclient.hpp"

using DownloadStatus = cortex::event::DownloadEventType;
class DownloadProgress {
 public:
  bool Connect(const std::string& host, int port);

  bool Handle(const std::string& id);

  void ForceStop() { force_stop_ = true; }

 private:
  bool should_stop() const {
    return (status_ != DownloadStatus::DownloadStarted &&
            status_ != DownloadStatus::DownloadUpdated) ||
           force_stop_;
  }

 private:
  std::unique_ptr<easywsclient::WebSocket> ws_;
  std::atomic<DownloadStatus> status_ = DownloadStatus::DownloadStarted;
  std::atomic<bool> force_stop_ = false;
};