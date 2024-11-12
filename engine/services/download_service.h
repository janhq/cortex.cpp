#pragma once

#include <curl/curl.h>
#include <eventpp/eventqueue.h>
#include <functional>
#include <optional>
#include <thread>
#include <unordered_set>
#include "common/download_task_queue.h"
#include "common/event.h"
#include "utils/result.hpp"

class DownloadService {
 private:
  static constexpr int MAX_CONCURRENT_TASKS = 4;

  struct DownloadingData {
    std::string task_id;
    std::string item_id;
    DownloadService* download_service;
  };

  // Each worker represents a thread. Each worker will have its own multi_handle
  struct WorkerData {
    CURLM* multi_handle;
    std::unordered_map<std::string, std::shared_ptr<DownloadingData>>
        downloading_data_map;
  };
  std::vector<std::unique_ptr<WorkerData>> worker_data_;

  std::vector<std::thread> worker_threads_;

  // Store all download tasks in a queue
  DownloadTaskQueue task_queue_;

  // Flag to stop Download service. Will stop all worker threads
  std::atomic<bool> stop_flag_{false};

  // Track active tasks across all workers
  std::mutex active_tasks_mutex_;
  std::unordered_map<std::string, std::shared_ptr<DownloadTask>> active_tasks_;

  // sync primitives
  std::condition_variable task_cv_;
  std::mutex task_mutex_;

  void WorkerThread(int worker_id);

  void ProcessTask(DownloadTask& task, int worker_id);

  cpp::result<void, std::string> ProcessMultiDownload(
      DownloadTask& task, CURLM* multi_handle,
      const std::vector<std::pair<CURL*, FILE*>>& handles);

  void SetUpCurlHandle(CURL* handle, const DownloadItem& item, FILE* file,
                       DownloadingData* dl_data);

  void EmitTaskStarted(const DownloadTask& task);

  void EmitTaskStopped(const std::string& task_id);

  void EmitTaskCompleted(const std::string& task_id);

  void EmitTaskError(const std::string& task_id);

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
    InitializeWorkers();
  };

  ~DownloadService() { Shutdown(); }

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
  void InitializeWorkers();

  void Shutdown();

  cpp::result<bool, std::string> Download(
      const std::string& download_id,
      const DownloadItem& download_item) noexcept;

  std::shared_ptr<EventQueue> event_queue_;

  CURLM* multi_handle_;
  std::thread worker_thread_;

  std::mutex queue_mutex_;
  std::condition_variable queue_condition_;

  // stop tasks
  std::unordered_set<std::string> tasks_to_stop_;
  std::mutex stop_mutex_;

  // callbacks
  std::unordered_map<std::string, std::function<void(const DownloadTask&)>>
      callbacks_;
  std::mutex callbacks_mutex_;

  void WorkerThread();

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

    if (downloading_data->download_service == nullptr) {
      return 0;
    }

    // Lock during the update and event emission
    std::lock_guard<std::mutex> lock(
        downloading_data->download_service->active_tasks_mutex_);

    // Find and update the task
    if (auto task_it = downloading_data->download_service->active_tasks_.find(
            downloading_data->task_id);
        task_it != downloading_data->download_service->active_tasks_.end()) {
      auto& task = task_it->second;
      // Find the specific item in the task
      for (auto& item : task->items) {
        if (item.id != downloading_data->item_id) {
          // not the item we are looking for
          continue;
        }

        if (dltotal == 0) {
          // if dltotal is 0, we prevent to send the event
          break;
        }

        item.bytes = dltotal;
        item.downloadedBytes = dlnow;

        static auto last_event_time = std::chrono::steady_clock::now();
        auto current_time = std::chrono::steady_clock::now();
        auto time_since_last_event =
            std::chrono::duration_cast<std::chrono::milliseconds>(
                current_time - last_event_time)
                .count();

        // throttle event by 1 sec
        if (time_since_last_event >= 1000) {
          downloading_data->download_service->event_queue_->enqueue(
              EventType::DownloadEvent,
              DownloadEvent{.type_ = DownloadEventType::DownloadUpdated,
                            .download_task_ = *task});

          // Update the last event time
          last_event_time = current_time;
        }

        break;
      }
    }

    return 0;
  }
};
