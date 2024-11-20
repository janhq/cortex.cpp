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
#include <sys/inotify.h>
#include <unistd.h>
#endif

class FileWatcherService {
 private:
#if defined(_WIN32)
  HANDLE dir_handle;
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
      : watch_path_{path}, running_{false} {
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
    if (fd >= 0) {
      close(fd);
    }
#endif

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
      if (eventFlags[i] & kFSEventStreamEventFlagItemRemoved) {
        watcher->model_service_->ForceIndexingModelList();
      }
    }
  }

  void WatcherThread() {
    // macOS implementation
    auto mypath = CFStringCreateWithCString(NULL, watch_path_.c_str(),
                                            kCFStringEncodingUTF8);
    auto path_to_watch = CFArrayCreate(NULL, (const void**)&mypath, 1, NULL);

    FSEventStreamContext context = {0, this, NULL, NULL, NULL};

    event_stream =
        FSEventStreamCreate(NULL, &FileWatcherService::callback, &context,
                            path_to_watch, kFSEventStreamEventIdSinceNow,
                            0.5,  // 500ms latency
                            kFSEventStreamCreateFlagFileEvents);

    dispatch_queue_t queue = dispatch_get_main_queue();
    FSEventStreamSetDispatchQueue(event_stream, queue);
    FSEventStreamStart(event_stream);

    while (running_) {
      CFRunLoopRunInMode(kCFRunLoopDefaultMode, 1.0, false);
    }

    FSEventStreamStop(event_stream);
    FSEventStreamInvalidate(event_stream);
    FSEventStreamRelease(event_stream);
    CFRelease(path_to_watch);
    CFRelease(mypath);
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
    wd = inotify_add_watch(fd, dirPath.c_str(),
                           IN_DELETE | IN_CREATE | IN_DELETE_SELF);
    if (wd < 0) {
      throw std::runtime_error("Failed to add watch on: " + dirPath);
    }
    watch_descriptors[wd] = dirPath;

    // Recursively add watches to subdirectories
    for (const auto& entry :
         std::filesystem::recursive_directory_iterator(dirPath)) {
      if (std::filesystem::is_directory(entry)) {
        AddWatch(entry.path().string());
      }
    }
  }

  void CleanupWatches() {
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
    fd = inotify_init();
    if (fd < 0) {
      throw std::runtime_error("Failed to initialize inotify");
    }

    // Add initial watch on the main directory
    AddWatch(watch_path_);

    char buffer[4096];
    while (running_) {
      int length = read(fd, buffer, sizeof(buffer));
      if (length < 0) {
        continue;
      }

      int i = 0;
      while (i < length) {
        struct inotify_event* event = (struct inotify_event*)&buffer[i];
        if (event->mask & IN_DELETE) {
          auto deletedPath = watch_descriptors[event->wd] + "/" + event->name;
          model_service_->ForceIndexingModelList();
        }
        i += sizeof(struct inotify_event) + event->len;
      }
    }

    close(fd);
  }
#endif
};
