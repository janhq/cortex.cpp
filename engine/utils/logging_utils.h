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
