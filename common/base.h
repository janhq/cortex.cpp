#pragma once
#include <drogon/HttpController.h>

using namespace drogon;

class BaseProvider {
public:
  virtual ~BaseProvider() {}

  // General inference method
  virtual void
  inference(const HttpRequestPtr &req,
            std::function<void(const HttpResponsePtr &)> &&callback) = 0;

  // Model management
  virtual void
  loadModel(const HttpRequestPtr &req,
            std::function<void(const HttpResponsePtr &)> &&callback) = 0;
  virtual void
  unloadModel(const HttpRequestPtr &req,
              std::function<void(const HttpResponsePtr &)> &&callback) = 0;
  virtual void
  modelStatus(const HttpRequestPtr &req,
              std::function<void(const HttpResponsePtr &)> &&callback) = 0;
};

class ChatProvider : public BaseProvider {
public:
  virtual ~ChatProvider() {}

  // Implement embedding functionality specific to chat
  virtual void
  embedding(const HttpRequestPtr &req,
            std::function<void(const HttpResponsePtr &)> &&callback) = 0;

  // The derived class can also override other methods if needed
};
