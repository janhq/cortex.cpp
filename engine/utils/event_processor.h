#pragma once

#include <eventpp/eventqueue.h>
#include <thread>
#include "common/download_event.h"
#include "common/event.h"

namespace cortex::event {

using Event = cortex::event::Event;
using DownloadEvent = cortex::event::DownloadEvent;
using EventQueue = eventpp::EventQueue<std::string, void(DownloadEvent)>;

class EventProcessor {
 public:
  EventProcessor(std::shared_ptr<EventQueue> queue)
      : event_queue_(std::move(queue)), running_(true) {
    thread_ = std::thread([this]() {
      while (running_) {
        event_queue_->wait();
        event_queue_->process();
      }
    });
  }

  ~EventProcessor() {
    running_ = false;
    if (thread_.joinable()) {
      thread_.join();
    }
  }

 private:
  std::shared_ptr<EventQueue> event_queue_;
  std::thread thread_;
  std::atomic<bool> running_;
};
}  // namespace cortex::event
