#pragma once

#include "trantor/utils/Logger.h"
// if verbose log all to console
// if not verbose only log result to console
inline bool log_verbose = false;

// CLI and Server are sharing logging utils, only print to message if executable is CLI
inline bool is_server = false;

// Only use trantor log
#define CTL_DBG(msg) LOG_DEBUG << msg;

#define CTL_INF(msg) LOG_INFO << msg;

#define CTL_WRN(msg) LOG_WARN << msg;

#define CTL_ERR(msg) LOG_ERROR << msg;

#define CLI_LOG(msg)               \
  if (log_verbose || is_server) {  \
    LOG_INFO << msg;               \
  } else {                         \
    std::cout << msg << std::endl; \
  }
#define CLI_LOG_ERROR(msg)         \
  if (log_verbose || is_server) {  \
    LOG_INFO << msg;               \
  } else {                         \
    LOG_ERROR << msg;              \
    std::cout << msg << std::endl; \
  }

namespace logging_utils_helper {
// In macOS, the default log level is reset to INFO when we load engine
// Use a global log level to save the value
inline trantor::Logger::LogLevel global_log_level = trantor::Logger::kInfo;
inline void SetLogLevel(const std::string& log_level, bool ignore_cout) {
  if (log_level == "TRACE") {
    trantor::Logger::setLogLevel(trantor::Logger::kTrace);
    global_log_level = trantor::Logger::kTrace;
    if (!ignore_cout)
      std::cout << "Set log level to TRACE" << std::endl;
  } else if (log_level == "DEBUG") {
    trantor::Logger::setLogLevel(trantor::Logger::kDebug);
    global_log_level = trantor::Logger::kDebug;
    if (!ignore_cout)
      std::cout << "Set log level to DEBUG" << std::endl;
  } else if (log_level == "INFO") {
    trantor::Logger::setLogLevel(trantor::Logger::kInfo);
    global_log_level = trantor::Logger::kInfo;
    if (!ignore_cout)
      std::cout << "Set log level to INFO" << std::endl;
  } else if (log_level == "WARN") {
    trantor::Logger::setLogLevel(trantor::Logger::kWarn);
    global_log_level = trantor::Logger::kWarn;
    if (!ignore_cout)
      std::cout << "Set log level to WARN" << std::endl;
  } else if (log_level == "ERROR") {
    trantor::Logger::setLogLevel(trantor::Logger::kError);
    global_log_level = trantor::Logger::kError;
    if (!ignore_cout)
      std::cout << "Set log level to ERROR" << std::endl;
  } else {
    std::cerr << "Invalid log level: " << log_level
              << ", loglevel must be (TRACE, DEBUG, INFO, WARN or ERROR)"
              << std::endl;
  }
}

inline std::string LogLevelStr(const trantor::Logger::LogLevel& log_level) {
  switch (log_level) {
    case trantor::Logger::kTrace:
      return "TRACE";
    case trantor::Logger::kDebug:
      return "DEBUG";
    case trantor::Logger::kInfo:
      return "INFO";
    case trantor::Logger::kWarn:
      return "WARN";
    case trantor::Logger::kError:
      return "ERROR";
    default:
      return "UNKNOWN";
  }
}
}  // namespace logging_utils_helper