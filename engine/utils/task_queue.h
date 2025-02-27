#pragma once
#include <memory>
#include <string>
#include "trantor/net/EventLoopThreadPool.h"

namespace cortex {
class TaskQueue {
 public:
  TaskQueue(size_t num_threads, const std::string& name)
      : ev_loop_pool_(
            std::make_unique<trantor::EventLoopThreadPool>(num_threads, name)) {
    ev_loop_pool_->start();
  }
  ~TaskQueue() {}

  template <typename Functor>
  void RunInQueue(Functor&& f) {
    if (ev_loop_pool_) {
      ev_loop_pool_->getNextLoop()->runInLoop(std::forward<Functor>(f));
    }
  }

  template <typename Functor>
  uint64_t RunEvery(const std::chrono::duration<double>& interval,
                    Functor&& cb) {
    if (ev_loop_pool_) {
      return ev_loop_pool_->getNextLoop()->runEvery(interval,
                                                    std::forward<Functor>(cb));
    }
    return 0;
  }

  template <typename Functor>
  uint64_t RunAfter(const std::chrono::duration<double>& delay, Functor&& cb) {
    if (ev_loop_pool_) {
      return ev_loop_pool_->getNextLoop()->runAfter(delay,
                                                    std::forward<Functor>(cb));
    }
    return 0;
  }

 private:
  std::unique_ptr<trantor::EventLoopThreadPool> ev_loop_pool_ = nullptr;
};
}  // namespace cortex