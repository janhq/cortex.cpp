#include <atomic>
#include <memory>
#include <string>
#include <thread>
#include "services/model_service.h"
#include "utils/logging_utils.h"

#ifdef __APPLE__
#include <CoreServices/CoreServices.h>
#include <sys/stat.h>

#elif defined(_WIN32)
#include <windows.h>

#else  // Linux
#include <limits.h>
#include <poll.h>
#include <sys/inotify.h>
#include <unistd.h>
#endif

class FileWatcherService {
 private:
#if defined(_WIN32)
  HANDLE dir_handle = INVALID_HANDLE_VALUE;
  HANDLE stop_event;
#elif defined(__APPLE__)
  FSEventStreamRef event_stream;
#else  // Linux
  int fd;
  int wd;
  std::unordered_map<int, std::string> watch_descriptors;
#endif

 public:
  FileWatcherService(const std::string& path,
                     std::shared_ptr<ModelService> model_service)
      : watch_path_{path}, running_{false}, model_service_{model_service} {
    if (!std::filesystem::exists(path)) {
      throw std::runtime_error("Path does not exist: " + path);
    }
#ifdef _WIN32
    stop_event = CreateEvent(NULL, TRUE, FALSE, NULL);
#endif
    CTL_INF("FileWatcherService created: " + path);
  }

  ~FileWatcherService() {
    CTL_INF("FileWatcherService destructor");
    stop();
  }

  void start() {
    if (running_) {
      return;
    }

    running_ = true;
    watch_thread_ = std::thread(&FileWatcherService::WatcherThread, this);
  }

  void stop() {
    if (!running_) {
      return;
    }

    running_ = false;

#ifdef _WIN32
    // Signal the stop event
    SetEvent(stop_event);
#elif defined(__APPLE__)
    if (event_stream) {
      FSEventStreamStop(event_stream);
      FSEventStreamInvalidate(event_stream);
    }
#else  // Linux
    // For Linux, closing the fd will interrupt the read() call
    CTL_INF("before close fd!");
    if (fd >= 0) {
      close(fd);
    }
#endif
    CTL_INF("before join!");
    // Add timeout to avoid infinite waiting
    if (watch_thread_.joinable()) {
      watch_thread_.join();
    }

#ifdef _WIN32
    if (stop_event != NULL) {
      CloseHandle(stop_event);
    }
    if (dir_handle != INVALID_HANDLE_VALUE) {
      CloseHandle(dir_handle);
    }
#elif defined(__APPLE__)
    if (event_stream) {
      FSEventStreamRelease(event_stream);
    }
#else  // Linux
    CleanupWatches();
#endif
    CTL_INF("FileWatcherService stopped!");
  }

 private:
  std::string watch_path_;
  std::atomic<bool> running_;
  std::thread watch_thread_;
  std::shared_ptr<ModelService> model_service_;

#ifdef __APPLE__

  static void callback(ConstFSEventStreamRef streamRef,
                       void* clientCallBackInfo, size_t numEvents,
                       void* eventPaths,
                       const FSEventStreamEventFlags eventFlags[],
                       const FSEventStreamEventId eventIds[]) {
    auto** paths = (char**)eventPaths;
    auto* watcher = static_cast<FileWatcherService*>(clientCallBackInfo);

    for (size_t i = 0; i < numEvents; i++) {
      if (eventFlags[i] & (kFSEventStreamEventFlagItemRemoved |
                           kFSEventStreamEventFlagItemRenamed |
                           kFSEventStreamEventFlagItemModified)) {
        CTL_INF("File removed: " + std::string(paths[i]));
        CTL_INF("File event detected: " + std::string(paths[i]) +
                " flags: " + std::to_string(eventFlags[i]));
        watcher->model_service_->ForceIndexingModelList();
      }
    }
  }

  void WatcherThread() {
    CFRunLoopRef runLoop = CFRunLoopGetCurrent();

    auto path = CFStringCreateWithCString(nullptr, watch_path_.c_str(),
                                          kCFStringEncodingUTF8);
    auto path_to_watch =
        CFArrayCreate(nullptr, (const void**)&path, 1, &kCFTypeArrayCallBacks);

    FSEventStreamContext context = {0, this, nullptr, nullptr, nullptr};

    event_stream = FSEventStreamCreate(
        nullptr, &FileWatcherService::callback, &context, path_to_watch,
        kFSEventStreamEventIdSinceNow, 1,  // each second
        kFSEventStreamCreateFlagFileEvents | kFSEventStreamCreateFlagNoDefer |
            kFSEventStreamCreateFlagWatchRoot);

    if (!event_stream) {
      CFRelease(path_to_watch);
      CFRelease(path);
      throw std::runtime_error("Failed to create FSEvent stream");
    }

    FSEventStreamScheduleWithRunLoop(event_stream, runLoop,
                                     kCFRunLoopDefaultMode);

    if (!FSEventStreamStart(event_stream)) {
      FSEventStreamInvalidate(event_stream);
      FSEventStreamRelease(event_stream);
      CFRelease(path_to_watch);
      CFRelease(path);
      throw std::runtime_error("Failed to start FSEvent stream");
    }

    while (running_) {
      CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.25, true);
    }

    FSEventStreamStop(event_stream);
    FSEventStreamInvalidate(event_stream);
    FSEventStreamRelease(event_stream);
    CFRelease(path_to_watch);
    CFRelease(path);
  }

#elif defined(_WIN32)
  void WatcherThread() {
    dir_handle =
        CreateFileA(watch_path_.c_str(), FILE_LIST_DIRECTORY,
                    FILE_SHARE_READ | FILE_SHARE_DELETE, NULL, OPEN_EXISTING,
                    FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, NULL);

    if (dir_handle == INVALID_HANDLE_VALUE) {
      throw std::runtime_error("Failed to open directory");
    }

    char buffer[4096];
    OVERLAPPED overlapped = {0};
    overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    DWORD bytesReturned;
    HANDLE events[] = {overlapped.hEvent, stop_event};
    while (running_) {
      if (!ReadDirectoryChangesW(
              dir_handle, buffer, sizeof(buffer), TRUE,
              FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME,
              &bytesReturned, &overlapped, NULL)) {
        break;
      }

      // Wait for either file change event or stop event
      DWORD result = WaitForMultipleObjects(2, events, FALSE, INFINITE);
      if (result == WAIT_OBJECT_0 + 1) {  // stop_event was signaled
        break;
      }

      if (result != WAIT_OBJECT_0 ||
          !GetOverlappedResult(dir_handle, &overlapped, &bytesReturned,
                               FALSE)) {
        break;
      }

      FILE_NOTIFY_INFORMATION* event = (FILE_NOTIFY_INFORMATION*)buffer;
      do {
        if (event->Action == FILE_ACTION_REMOVED) {
          std::wstring fileName(event->FileName,
                                event->FileNameLength / sizeof(wchar_t));

          std::string file_name_str(fileName.begin(), fileName.end());
          model_service_->ForceIndexingModelList();
        }

        if (event->NextEntryOffset == 0)
          break;
        event = (FILE_NOTIFY_INFORMATION*)((uint8_t*)event +
                                           event->NextEntryOffset);
      } while (true);

      ResetEvent(overlapped.hEvent);
    }

    CloseHandle(overlapped.hEvent);
    CloseHandle(dir_handle);
  }

#else  // Linux

  void AddWatch(const std::string& dirPath) {
    const int watch_flags = IN_DELETE | IN_DELETE_SELF | IN_CREATE;
    wd = inotify_add_watch(fd, dirPath.c_str(), watch_flags);
    if (wd < 0) {
      throw std::runtime_error("Failed to add watch on " + dirPath + ": " +
                               std::string(strerror(errno)));
    }
    watch_descriptors[wd] = dirPath;

    // Add watches for subdirectories
    try {
      for (const auto& entry :
           std::filesystem::recursive_directory_iterator(dirPath)) {
        if (std::filesystem::is_directory(entry)) {
          int subwd = inotify_add_watch(fd, entry.path().c_str(), watch_flags);
          if (subwd >= 0) {
            watch_descriptors[subwd] = entry.path().string();
          } else {
            CTL_ERR("Failed to add watch for subdirectory " +
                    entry.path().string() + ": " +
                    std::string(strerror(errno)));
          }
        }
      }
    } catch (const std::filesystem::filesystem_error& e) {
      CTL_ERR("Error walking directory tree: " + std::string(e.what()));
    }
  }

  void CleanupWatches() {
    CTL_INF("Cleanup Watches");
    for (const auto& [wd, path] : watch_descriptors) {
      inotify_rm_watch(fd, wd);
    }
    watch_descriptors.clear();

    if (fd >= 0) {
      close(fd);
      fd = -1;
    }
  }

  void WatcherThread() {
    fd = inotify_init1(IN_NONBLOCK);
    if (fd < 0) {
      CTL_ERR("Failed to initialize inotify: " + std::string(strerror(errno)));
      return;
    }

    try {
      AddWatch(watch_path_);
    } catch (const std::exception& e) {
      CTL_ERR("Failed to add watch: " + std::string(e.what()));
      close(fd);
      return;
    }

    const int POLL_TIMEOUT_MS = 1000;  // 1 second timeout
    char buffer[4096];
    struct pollfd pfd = {fd, POLLIN, 0};

    while (running_) {
      // Poll will sleep until either:
      // 1. Events are available (POLLIN)
      // 2. POLL_TIMEOUT_MS milliseconds have elapsed
      // 3. An error occurs
      int poll_result = poll(&pfd, 1, POLL_TIMEOUT_MS);

      if (poll_result < 0) {
        if (errno == EINTR) {
          // System call was interrupted, just retry
          continue;
        }
        CTL_ERR("Poll failed: " + std::string(strerror(errno)));
        break;
      }

      if (poll_result == 0) {  // Timeout - no events
        // No need to sleep - poll() already waited
        continue;
      }

      if (pfd.revents & POLLERR || pfd.revents & POLLNVAL) {
        CTL_ERR("Poll error on fd");
        break;
      }

      // Read all pending events
      while (running_) {
        int length = read(fd, buffer, sizeof(buffer));
        if (length < 0) {
          if (errno == EAGAIN || errno == EWOULDBLOCK) {
            // No more events to read
            break;
          }
          CTL_ERR("Read error: " + std::string(strerror(errno)));
          break;
        }

        if (length == 0) {
          break;
        }

        // Process events
        size_t i = 0;
        while (i < static_cast<size_t>(length)) {
          struct inotify_event* event =
              reinterpret_cast<struct inotify_event*>(&buffer[i]);

          if (event->mask & (IN_DELETE | IN_DELETE_SELF)) {
            try {
              model_service_->ForceIndexingModelList();
            } catch (const std::exception& e) {
              CTL_ERR("Error processing delete event: " +
                      std::string(e.what()));
            }
          }

          i += sizeof(struct inotify_event) + event->len;
        }
      }
    }
  }
#endif
};
