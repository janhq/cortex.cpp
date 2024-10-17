#pragma once

#include "trantor/utils/Logger.h"
// if verbose log all to console
// if not verbose only log result to console
inline bool log_verbose = false;

// Only use trantor log
#define CTL_DBG(msg)  \
  if (!log_verbose) { \
    LOG_DEBUG << msg; \
  }

#define CTL_INF(msg)  \
  if (!log_verbose) { \
    LOG_INFO << msg;  \
  }

#define CTL_WRN(msg)  \
  if (!log_verbose) { \
    LOG_WARN << msg;  \
  }

// Use std::cout if not verbose, use trantor log if verbose
#define CTL_ERR(msg) LOG_ERROR << msg;

#define CLI_LOG(msg)               \
  if (log_verbose) {               \
    LOG_INFO << msg;               \
  } else {                         \
    std::cout << msg << std::endl; \
  }
#define CLI_LOG_ERROR(msg)         \
  if (log_verbose) {               \
    LOG_INFO << msg;               \
  } else {                         \
    LOG_ERROR << msg;              \
    std::cout << msg << std::endl; \
  }
