#pragma once

#include <eventpp/eventqueue.h>
#include <thread>
#include "common/event.h"

namespace cortex::event {

using EventType = cortex::event::EventType;
using ExitEvent = cortex::event::ExitEvent;
using EventQueue =
    eventpp::EventQueue<EventType, void(const eventpp::AnyData<eventMaxSize>&)>;

struct ExitingEvent {};

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
    // to prevent blocking thread on wait
    event_queue_->enqueue(EventType::ExitEvent,
                          ExitEvent{{}, "Event queue exitting.."});
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
