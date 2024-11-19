#include <atomic>
#include <iostream>
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
 public:
  FileWatcherService(const std::string& path,
                     std::shared_ptr<ModelService> model_service)
      : watchPath{path}, running{false} {
    CTL_INF("FileWatcherService created: " + path);
  }

  ~FileWatcherService() { stop(); }

  void start() {
    if (running)
      return;
    running = true;
    watchThread = std::thread(&FileWatcherService::watcherThread, this);
  }

  void stop() {
    CTL_INF("FileWatcherService stop");
    running = false;
    if (watchThread.joinable()) {
      watchThread.join();
    }
  }

 private:
  std::string watchPath;
  std::atomic<bool> running;
  std::thread watchThread;
  std::shared_ptr<ModelService> model_service_;

#ifdef __APPLE__
  static void callback(ConstFSEventStreamRef streamRef,
                       void* clientCallBackInfo, size_t numEvents,
                       void* eventPaths,
                       const FSEventStreamEventFlags eventFlags[],
                       const FSEventStreamEventId eventIds[]) {
    char** paths = (char**)eventPaths;
    // model_service->ForceIndexingModelList();
    FileWatcherService* watcher =
        static_cast<FileWatcherService*>(clientCallBackInfo);
    watcher->model_service_->ForceIndexingModelList();
    for (size_t i = 0; i < numEvents; i++) {
      if (eventFlags[i] & kFSEventStreamEventFlagItemRemoved) {
        std::cout << "File deleted: " << paths[i] << std::endl;
      }
    }
  }

  void watcherThread() {
    FSEventStreamContext context = {0, this, nullptr, nullptr, nullptr};
    CFStringRef pathRef = CFStringCreateWithCString(nullptr, watchPath.c_str(),
                                                    kCFStringEncodingUTF8);
    CFArrayRef pathsToWatch =
        CFArrayCreate(nullptr, (const void**)&pathRef, 1, nullptr);

    FSEventStreamRef stream =
        FSEventStreamCreate(nullptr, &callback, &context, pathsToWatch,
                            kFSEventStreamEventIdSinceNow,
                            0.5,  // 500ms latency
                            kFSEventStreamCreateFlagFileEvents);

    FSEventStreamScheduleWithRunLoop(stream, CFRunLoopGetCurrent(),
                                     kCFRunLoopDefaultMode);
    FSEventStreamStart(stream);

    CTL_INF("NamH start loop");
    while (running) {
      CFRunLoopRunInMode(kCFRunLoopDefaultMode, 1.0, false);
    }

    FSEventStreamStop(stream);
    FSEventStreamUnscheduleFromRunLoop(stream, CFRunLoopGetCurrent(),
                                       kCFRunLoopDefaultMode);
    FSEventStreamInvalidate(stream);
    FSEventStreamRelease(stream);
    CFRelease(pathsToWatch);
    CFRelease(pathRef);
  }

#elif defined(_WIN32)
  void watcherThread() {
    HANDLE hDir =
        CreateFileA(watchPath.c_str(), FILE_LIST_DIRECTORY,
                    FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                    nullptr, OPEN_EXISTING,
                    FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, nullptr);

    if (hDir == INVALID_HANDLE_VALUE) {
      std::cerr << "Failed to open directory" << std::endl;
      return;
    }

    char buffer[4096];
    DWORD bytesReturned;
    OVERLAPPED overlapped = {0};
    overlapped.hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);

    while (running) {
      ReadDirectoryChangesW(
          hDir, buffer, sizeof(buffer), TRUE,
          FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME,
          &bytesReturned, &overlapped, nullptr);

      WaitForSingleObject(overlapped.hEvent, 1000);

      FILE_NOTIFY_INFORMATION* event = (FILE_NOTIFY_INFORMATION*)buffer;
      do {
        if (event->Action == FILE_ACTION_REMOVED) {
          wchar_t fileName[MAX_PATH];
          memcpy(fileName, event->FileName, event->FileNameLength);
          fileName[event->FileNameLength / 2] = L'\0';
          std::wcout << L"File deleted: " << fileName << std::endl;
        }

        if (event->NextEntryOffset == 0)
          break;
        event = (FILE_NOTIFY_INFORMATION*)((uint8_t*)event +
                                           event->NextEntryOffset);
      } while (true);

      ResetEvent(overlapped.hEvent);
    }

    CloseHandle(overlapped.hEvent);
    CloseHandle(hDir);
  }

#else  // Linux
  void watcherThread() {
    int fd = inotify_init();
    if (fd < 0) {
      std::cerr << "Failed to initialize inotify" << std::endl;
      return;
    }

    int wd = inotify_add_watch(fd, watchPath.c_str(), IN_DELETE);
    if (wd < 0) {
      std::cerr << "Failed to add watch" << std::endl;
      close(fd);
      return;
    }

    const size_t event_size = sizeof(struct inotify_event);
    const size_t buf_len = 1024 * (event_size + 16);
    char buffer[buf_len];

    while (running) {
      int length = read(fd, buffer, buf_len);
      if (length < 0) {
        if (errno == EINTR)
          continue;
        break;
      }

      int i = 0;
      while (i < length) {
        struct inotify_event* event = (struct inotify_event*)&buffer[i];
        if (event->len && (event->mask & IN_DELETE)) {
          std::cout << "File deleted: " << event->name << std::endl;
        }
        i += event_size + event->len;
      }
    }

    inotify_rm_watch(fd, wd);
    close(fd);
  }
#endif
};
