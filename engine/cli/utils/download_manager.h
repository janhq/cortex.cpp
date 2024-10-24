#pragma once
#include <atomic>
#include <memory>
#include <string>
#include "common/event.h"
#include "easywsclient.hpp"

using DownloadStatus = cortex::event::DownloadEventType;
class DownloadManager {
 public:
  bool Connect(const std::string& host, int port);

  bool Handle(const std::string& id);

 private:
  bool should_stop() const {
    return status_ != DownloadStatus::DownloadStarted &&
           status_ != DownloadStatus::DownloadUpdated;
  }

 private:
  // TODO(sang) open multiple sockets
  std::unique_ptr<easywsclient::WebSocket> ws_;
  std::atomic<DownloadStatus> status_ = DownloadStatus::DownloadStarted;
};