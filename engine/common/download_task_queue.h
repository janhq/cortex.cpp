#include <condition_variable>
#include <deque>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include "common/download_task.h"
#include <algorithm>

class DownloadTaskQueue {
 private:
  std::deque<DownloadTask> taskQueue;
  std::unordered_map<std::string, typename std::deque<DownloadTask>::iterator>
      taskMap;
  mutable std::shared_mutex mutex;
  std::condition_variable_any cv;

 public:
  void push(DownloadTask task) {
    std::unique_lock lock(mutex);
    taskQueue.push_back(std::move(task));
    taskMap[taskQueue.back().id] = std::prev(taskQueue.end());
    cv.notify_one();
  }

  std::optional<DownloadTask> pop() {
    std::unique_lock lock(mutex);
    if (taskQueue.empty()) {
      return std::nullopt;
    }
    DownloadTask task = std::move(taskQueue.front());
    taskQueue.pop_front();
    taskMap.erase(task.id);
    return task;
  }

  bool cancelTask(const std::string& taskId) {
    std::unique_lock lock(mutex);
    auto it = taskMap.find(taskId);
    if (it != taskMap.end()) {
      it->second->status = DownloadTask::Status::Cancelled;
      taskQueue.erase(it->second);
      taskMap.erase(it);
      return true;
    }
    return false;
  }

  bool updateTaskStatus(const std::string& taskId,
                        DownloadTask::Status newStatus) {
    std::unique_lock lock(mutex);
    auto it = taskMap.find(taskId);
    if (it != taskMap.end()) {
      it->second->status = newStatus;
      if (newStatus == DownloadTask::Status::Cancelled ||
          newStatus == DownloadTask::Status::Error) {
        taskQueue.erase(it->second);
        taskMap.erase(it);
      }
      return true;
    }
    return false;
  }

  std::optional<DownloadTask> getNextPendingTask() {
    std::shared_lock lock(mutex);
    auto it = std::find_if(
        taskQueue.begin(), taskQueue.end(), [](const DownloadTask& task) {
          return task.status == DownloadTask::Status::Pending;
        });

    if (it != taskQueue.end()) {
      return *it;
    }
    return std::nullopt;
  }
};
