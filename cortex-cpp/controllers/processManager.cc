#include "processManager.h"
#include <cstdlib>
#include <trantor/utils/Logger.h>

void processManager::destroy(
    const HttpRequestPtr &req,
    std::function<void(const HttpResponsePtr &)> &&callback) {
  LOG_INFO << "Program is exitting, goodbye!";
  exit(0);
  return;
};
