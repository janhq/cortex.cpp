#include "file_logger.h"
#include <algorithm>
#include <iostream>
#include <sstream>

#ifdef _WIN32
#include <io.h>
#define ftruncate _chsize
#else
#include <unistd.h>
#endif
#include <string.h>

using namespace trantor;

FileLogger::FileLogger() : AsyncFileLogger() {}

FileLogger::~FileLogger() = default;

void FileLogger::output_(const char* msg, const uint64_t len) {
  if (!circular_log_file_ptr_) {
    circular_log_file_ptr_ =
        std::make_unique<CircularLogFile>(fileBaseName_, max_lines_);
  }
  circular_log_file_ptr_->writeLog(msg, len);
}

FileLogger::CircularLogFile::CircularLogFile(const std::string& fileName,
                                             uint64_t maxLines)
    : max_lines_(maxLines), file_name_(fileName) {
  std::lock_guard<std::mutex> lock(mutex_);
  OpenFile();
  LoadExistingLines();
  TruncateFileIfNeeded();
}

FileLogger::CircularLogFile::~CircularLogFile() {
  std::lock_guard<std::mutex> lock(mutex_);
  CloseFile();
}
void FileLogger::CircularLogFile::writeLog(const char* logLine,
                                           const uint64_t len) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (!fp_)
    return;

  std::string logString(logLine, len);
  std::istringstream iss(logString);
  std::string line;
  while (std::getline(iss, line)) {
    if (lineBuffer_.size() >= max_lines_) {
      lineBuffer_.pop_front();
    }
    lineBuffer_.push_back(line);
    AppendToFile(line + "\n");
    ++linesWrittenSinceLastTruncate_;
    if (static_cast<uint64_t>(linesWrittenSinceLastTruncate_.load()) >= TRUNCATE_CHECK_INTERVAL) {

      TruncateFileIfNeeded();
    }
  }
}
void FileLogger::CircularLogFile::flush() {
  std::lock_guard<std::mutex> lock(mutex_);
  if (fp_) {
    fflush(fp_);
  }
}

void FileLogger::CircularLogFile::TruncateFileIfNeeded() {
  // std::cout<<"Truncating file "<< totalLines_ <<std::endl;
  if (!fp_ || lineBuffer_.size() < max_lines_)
    return;

  // Close the current file
  fclose(fp_);
  fp_ = nullptr;

  // Open a temporary file for writing
  std::string tempFileName = file_name_ + ".temp";
  FILE* tempFile = fopen(tempFileName.c_str(), "w");
  if (!tempFile) {

    std::cout << "Error opening temporary file for truncation: "
              << strerror(errno) << std::endl;
    OpenFile();  // Reopen the original file
    return;
  }

  // Write only the last max_lines_ lines to the temporary file
  size_t startIndex =
      lineBuffer_.size() > max_lines_ ? lineBuffer_.size() - max_lines_ : 0;

  for (size_t i = startIndex; i < lineBuffer_.size(); ++i) {
    fprintf(tempFile, "%s\n", lineBuffer_[i].c_str());
  }

  fclose(tempFile);

  // Replace the original file with the temporary file
  if (std::rename(tempFileName.c_str(), file_name_.c_str()) != 0) {
    std::cout << "Error replacing original file with truncated file: "
              << strerror(errno) << std::endl;
    std::remove(tempFileName.c_str());  // Clean up the temporary file
  }
  //    else {
  //     totalLines_.store(lineBuffer_.size() > max_lines_ ? max_lines_
  //                                                       : lineBuffer_.size());
  //   }

  // Reopen the file
  OpenFile();
  // LoadExistingLines();
  linesWrittenSinceLastTruncate_.store(0);
}

void FileLogger::CircularLogFile::OpenFile() {
#ifdef _WIN32
  auto wFileName = utils::toNativePath(file_name_);
  fp_ = _wfopen(wFileName.c_str(), L"a+");
#else
  fp_ = fopen(file_name_.c_str(), "a+");
#endif

  if (!fp_) {
// If file doesn't exist, create it
#ifdef _WIN32
    fp_ = _wfopen(wFileName.c_str(), L"w+");
#else
    fp_ = fopen(file_name_.c_str(), "w+");
#endif

    if (!fp_) {
      std::cerr << "Error opening file: " << strerror(errno) << std::endl;
    }
  }
}
void FileLogger::CircularLogFile::LoadExistingLines() {
  if (!fp_)
    return;

  // Move to the beginning of the file
  fseek(fp_, 0, SEEK_SET);

  lineBuffer_.clear();

  std::string line;
  char buffer[4096];
  while (fgets(buffer, sizeof(buffer), fp_) != nullptr) {
    line = buffer;
    if (!line.empty() && line.back() == '\n') {
      line.pop_back();  // Remove trailing newline
    }
    if (lineBuffer_.size() >= max_lines_) {
      lineBuffer_.pop_front();
    }
    lineBuffer_.push_back(line);
  }

  // Move back to the end of the file for appending
  fseek(fp_, 0, SEEK_END);
}
void FileLogger::CircularLogFile::AppendToFile(const std::string& line) {
  if (fp_) {
    fwrite(line.c_str(), 1, line.length(), fp_);
    fflush(fp_);
  }
}

void FileLogger::CircularLogFile::CloseFile() {
  if (fp_) {
    fclose(fp_);
    fp_ = nullptr;
  }
}