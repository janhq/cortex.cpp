#pragma once

#include <json/value.h>
#include <condition_variable>
#include <mutex>
#include <queue>

// Status and result
using InferResult = std::pair<Json::Value, Json::Value>;

struct SyncQueue {
  void push(InferResult&& p) {
    std::unique_lock<std::mutex> l(mtx);
    q.push(p);
    cond.notify_one();
  }

  InferResult wait_and_pop() {
    std::unique_lock<std::mutex> l(mtx);
    cond.wait(l, [this] { return !q.empty(); });
    auto res = q.front();
    q.pop();
    return res;
  }

  std::mutex mtx;
  std::condition_variable cond;
  std::queue<InferResult> q;
};
