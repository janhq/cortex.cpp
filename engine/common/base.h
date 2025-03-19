#pragma once
#include <drogon/HttpController.h>

using namespace drogon;

class BaseModel {
 public:
  virtual ~BaseModel() = default;

  // Model management
  virtual void LoadModel(
      const HttpRequestPtr& req,
      std::function<void(const HttpResponsePtr&)>&& callback) = 0;
  virtual void UnloadModel(
      const HttpRequestPtr& req,
      std::function<void(const HttpResponsePtr&)>&& callback) = 0;
  virtual void ModelStatus(
      const HttpRequestPtr& req,
      std::function<void(const HttpResponsePtr&)>&& callback) = 0;
  virtual void GetModels(
      const HttpRequestPtr& req,
      std::function<void(const HttpResponsePtr&)>&& callback) = 0;
};

class BaseChatCompletion {
 public:
  virtual ~BaseChatCompletion() = default;

  // General chat method
  virtual void ChatCompletion(
      const HttpRequestPtr& req,
      std::function<void(const HttpResponsePtr&)>&& callback) = 0;
};

class BaseEmbedding {
 public:
  virtual ~BaseEmbedding() = default;

  // Implement embedding functionality specific to chat
  virtual void Embedding(
      const HttpRequestPtr& req,
      std::function<void(const HttpResponsePtr&)>&& callback) = 0;

  // The derived class can also override other methods if needed
};

