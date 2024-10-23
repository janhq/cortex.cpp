#pragma once

#include <curl/curl.h>
#include <eventpp/eventqueue.h>
#include <filesystem>
#include <functional>
#include <optional>
#include <queue>
#include <thread>
#include <unordered_set>
#include "common/event.h"
#include "utils/logging_utils.h"
#include "utils/result.hpp"

class DownloadService {
 public:
  using OnDownloadTaskSuccessfully =
      std::function<void(const DownloadTask& task)>;

  using DownloadEventType = cortex::event::DownloadEventType;
  using DownloadEvent = cortex::event::DownloadEvent;
  using EventType = cortex::event::EventType;
  using EventQueue =
      eventpp::EventQueue<EventType,
                          void(const eventpp::AnyData<eventMaxSize>&)>;

  explicit DownloadService() = default;

  explicit DownloadService(std::shared_ptr<EventQueue> event_queue)
      : event_queue_{event_queue} {
    curl_global_init(CURL_GLOBAL_ALL);
    stop_flag_ = false;
    multi_handle_ = curl_multi_init();
    worker_thread_ = std::thread(&DownloadService::WorkerThread, this);
  };

  ~DownloadService() {
    if (event_queue_ != nullptr) {
      stop_flag_ = true;
      queue_condition_.notify_one();

      CTL_INF("DownloadService is being destroyed.");
      curl_multi_cleanup(multi_handle_);
      curl_global_cleanup();

      if (worker_thread_.joinable()) {
        worker_thread_.join();
      }
      CTL_INF("DownloadService is destroyed.");
    }
  }

  DownloadService(const DownloadService&) = delete;
  DownloadService& operator=(const DownloadService&) = delete;

  /**
   * Adding new download task to the queue. Asynchronously. This function should 
   * be used by HTTP API.
   */
  cpp::result<DownloadTask, std::string> AddTask(
      DownloadTask& task, std::function<void(const DownloadTask&)> callback);

  /**
   * Start download task synchronously.
   */
  cpp::result<bool, std::string> AddDownloadTask(
      DownloadTask& task, std::optional<OnDownloadTaskSuccessfully> callback =
                              std::nullopt) noexcept;

  /**
   * Getting file size for a provided url. Can be used to validating the download url.
   *
   * @param url - url to get file size
   */
  cpp::result<uint64_t, std::string> GetFileSize(
      const std::string& url) const noexcept;

  cpp::result<std::string, std::string> StopTask(const std::string& task_id);

 private:
  struct DownloadingData {
    std::string item_id;
    DownloadService* download_service;
  };

  cpp::result<void, std::string> VerifyDownloadTask(
      DownloadTask& task) const noexcept;

  cpp::result<bool, std::string> Download(
      const std::string& download_id,
      const DownloadItem& download_item) noexcept;

  curl_off_t GetLocalFileSize(const std::filesystem::path& path) const;

  std::shared_ptr<EventQueue> event_queue_;

  CURLM* multi_handle_;
  std::thread worker_thread_;
  std::atomic<bool> stop_flag_;

  // task queue
  std::queue<DownloadTask> task_queue_;
  std::mutex queue_mutex_;
  std::condition_variable queue_condition_;

  // stop tasks
  std::unordered_set<std::string> tasks_to_stop_;
  std::mutex stop_mutex_;

  // callbacks
  std::unordered_map<std::string, std::function<void(const DownloadTask&)>>
      callbacks_;
  std::mutex callbacks_mutex_;

  std::shared_ptr<DownloadTask> active_task_;
  std::unordered_map<std::string, std::shared_ptr<DownloadingData>>
      downloading_data_map_;

  void WorkerThread();

  void ProcessCompletedTransfers();

  void ProcessTask(DownloadTask& task);

  bool IsTaskTerminated(const std::string& task_id);

  void RemoveTaskFromStopList(const std::string& task_id);

  void ExecuteCallback(const DownloadTask& task);

  constexpr static auto MAX_WAIT_MSECS = 1000;

  static int ProgressCallback(void* ptr, curl_off_t dltotal, curl_off_t dlnow,
                              curl_off_t ultotal, curl_off_t ulnow) {
    auto downloading_data = static_cast<DownloadingData*>(ptr);
    if (downloading_data == nullptr) {
      return 0;
    }
    const auto dl_item_id = downloading_data->item_id;
    if (dltotal <= 0) {
      return 0;
    }

    auto dl_srv = downloading_data->download_service;
    auto active_task = dl_srv->active_task_;
    if (active_task == nullptr) {
      return 0;
    }

    for (auto& item : active_task->items) {
      if (item.id == dl_item_id) {
        item.downloadedBytes = dlnow;
        item.bytes = dltotal;
        break;
      }
    }

    // Check if one second has passed since the last event
    static auto last_event_time = std::chrono::steady_clock::now();
    auto current_time = std::chrono::steady_clock::now();
    auto time_since_last_event =
        std::chrono::duration_cast<std::chrono::milliseconds>(current_time -
                                                              last_event_time)
            .count();

    // throttle event by 1 sec
    if (time_since_last_event >= 1000) {
      dl_srv->event_queue_->enqueue(
          EventType::DownloadEvent,
          DownloadEvent{.type_ = DownloadEventType::DownloadUpdated,
                        .download_task_ = *active_task});

      // Update the last event time
      last_event_time = current_time;
    }

    return 0;
  }
};
