#include <condition_variable>
#include <deque>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include "common/download_task.h"

class DownloadTaskQueue {
 private:
  std::deque<DownloadTask> task_queue_;
  std::unordered_map<std::string, typename std::deque<DownloadTask>::iterator>
      task_map_;
  mutable std::shared_mutex mtx_;
  std::condition_variable_any cv_;

 public:
  void Push(DownloadTask task) {
    std::unique_lock lock(mtx_);
    task_queue_.push_back(std::move(task));
    task_map_[task_queue_.back().id] = std::prev(task_queue_.end());
    cv_.notify_one();
  }

  std::optional<DownloadTask> Pop() {
    std::unique_lock lock(mtx_);
    if (task_queue_.empty()) {
      return std::nullopt;
    }
    DownloadTask task = std::move(task_queue_.front());
    task_queue_.pop_front();
    task_map_.erase(task.id);
    return task;
  }

  bool CancelTask(const std::string& task_id) {
    std::unique_lock lock(mtx_);
    auto it = task_map_.find(task_id);
    if (it != task_map_.end()) {
      it->second->status = DownloadTask::Status::Cancelled;
      task_queue_.erase(it->second);
      task_map_.erase(it);
      return true;
    }
    return false;
  }

  bool UpdateTaskStatus(const std::string& task_id,
                        DownloadTask::Status newStatus) {
    std::unique_lock lock(mtx_);
    auto it = task_map_.find(task_id);
    if (it != task_map_.end()) {
      it->second->status = newStatus;
      if (newStatus == DownloadTask::Status::Cancelled ||
          newStatus == DownloadTask::Status::Error) {
        task_queue_.erase(it->second);
        task_map_.erase(it);
      }
      return true;
    }
    return false;
  }

  std::optional<DownloadTask> GetNextPendingTask() {
    std::shared_lock lock(mtx_);
    auto it = std::find_if(
        task_queue_.begin(), task_queue_.end(), [](const DownloadTask& task) {
          return task.status == DownloadTask::Status::Pending;
        });

    if (it != task_queue_.end()) {
      return *it;
    }
    return std::nullopt;
  }
};
