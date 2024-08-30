#include <napi.h>

#include <drogon/HttpAppFramework.h>
#include <drogon/drogon.h>
#include <stdlib.h>
#include <climits>  // for PATH_MAX
#include <iostream>
#include "cortex-common/cortexpythoni.h"
#include "utils/cortex_utils.h"
#include "utils/dylib.h"

#if defined(__APPLE__) && defined(__MACH__)
#include <libgen.h>  // for dirname()
#include <mach-o/dyld.h>
#elif defined(__linux__)
#include <libgen.h>  // for dirname()
#include <unistd.h>  // for readlink()
#elif defined(_WIN32)
#include <windows.h>
#undef max
#else
#error "Unsupported platform!"
#endif

static Napi::Env* s_env = nullptr;

void start(const int port = 3929) {
  int thread_num = 1;
  std::string host = "127.0.0.1";
  int logical_cores = std::thread::hardware_concurrency();
  int drogon_thread_num = std::max(thread_num, logical_cores);
#ifdef CORTEX_CPP_VERSION
  LOG_INFO << "cortex-cpp version: " << CORTEX_CPP_VERSION;
#else
  LOG_INFO << "cortex-cpp version: undefined";
#endif

  LOG_INFO << "Server started, listening at: " << host << ":" << port;
  LOG_INFO << "Please load your model";
  drogon::app().addListener(host, port);
  drogon::app().setThreadNum(drogon_thread_num);
  LOG_INFO << "Number of thread is:" << drogon::app().getThreadNum();

  drogon::app().run();
}

void stop() {
  drogon::app().quit();
}

void exitCallback() {
  Napi::TypeError::New(*s_env, "Process Exited!").ThrowAsJavaScriptException();
}

Napi::Value Start(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  s_env = &env;

  // Register exitCallback with atexit
  std::atexit(exitCallback);


  Napi::Number jsParam = info[0].As<Napi::Number>();
  int port = jsParam.Int32Value();

  start(port);
  return env.Undefined();
}

Napi::Value Stop(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  stop();
  return Napi::String::New(env, "Server stopped successfully");
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  exports.Set(Napi::String::New(env, "start"), Napi::Function::New(env, Start));
  exports.Set(Napi::String::New(env, "stop"), Napi::Function::New(env, Start));
  return exports;
}

NODE_API_MODULE(cortex-cpp, Init)
