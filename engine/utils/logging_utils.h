#pragma once

#include "trantor/utils/Logger.h"
// if verbose log all to console
// if not verbose only log result to console
inline bool log_verbose = false;

#define CTL_INF(msg) \
  if (log_verbose) {    \
    LOG_INFO << msg;    \
  }

#define CTL_WRN(msg) \
  if (log_verbose) {    \
    LOG_WARN << msg;    \
  }

#define CTL_ERR(msg)           \
  if (log_verbose) {               \
    LOG_ERROR << msg;              \
  } else {                         \
    std::cout << msg << std::endl; \
  }

#define CLI_LOG(msg)               \
  if (log_verbose) {               \
    LOG_INFO << msg;               \
  } else {                         \
    std::cout << msg << std::endl; \
  }