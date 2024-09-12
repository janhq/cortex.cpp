#pragma once

#include <trantor/utils/AsyncFileLogger.h>
#include <trantor/utils/Utilities.h>
#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <mutex>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace trantor {

class TRANTOR_EXPORT FileLogger : NonCopyable {
 public:
  FileLogger();
  ~FileLogger();

  /**
     * @brief Set the maximum number of lines to keep in the log file.
     *
     * @param maxLines
     */
  void setMaxLines(uint64_t maxLines) { max_lines_ = maxLines; }

  /**
     * @brief Set the log file name.
     *
     * @param fileName The full name of the log file.
     */
  void setFileName(const std::string& fileName) {
    filePath_ = "./";
    fileBaseName_ = fileName;
    fileExtName_ = "";
  }
  void output_(const char* msg, const uint64_t len);
  std::string filePath_;
  std::string fileBaseName_;
  std::string fileExtName_;
  void startLogging();
  void flush_();

 protected:
  mutable std::mutex mutex_;
  std::condition_variable cond_;
  std::unique_ptr<std::thread> thread_ptr_;
  bool stop_flag_{false};
  void LogThreadFunc();

  class CircularLogFile {
   public:
    CircularLogFile(const std::string& fileName, uint64_t maxLines);
    ~CircularLogFile();

    void writeLog();
    void flush();
    uint64_t getLength() const { return lineBuffer_.size(); }
    void AddLineBuffer(const char* logLine, const uint64_t len);
   protected:
    FILE* fp_{nullptr};
    uint64_t max_lines_;
    std::string file_name_;

    std::atomic<int> linesWrittenSinceLastTruncate_{0};
    static const uint64_t TRUNCATE_CHECK_INTERVAL = 1000;
    std::deque<std::string> tmpBuffer_;
    std::deque<std::string> lineBuffer_;
    void LoadExistingLines();
    void TruncateFileIfNeeded();
    void AppendToFile(const std::string& line);
    void OpenFile();
    void CloseFile();
  };
  std::unique_ptr<CircularLogFile> circular_log_file_ptr_;
  uint64_t max_lines_{100000};  // Default to 100000 lines
};

}  // namespace trantor