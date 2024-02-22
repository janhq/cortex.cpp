#pragma once
#include <drogon/HttpController.h>

using namespace drogon;

class BaseModel {
 public:
  virtual ~BaseModel() {}

  // Model management
  virtual void LoadModel(
      const HttpRequestPtr &req,
      std::function<void(const HttpResponsePtr &)> &&callback) = 0;
  virtual void UnloadModel(
      const HttpRequestPtr &req,
      std::function<void(const HttpResponsePtr &)> &&callback) = 0;
  virtual void ModelStatus(
      const HttpRequestPtr &req,
      std::function<void(const HttpResponsePtr &)> &&callback) = 0;
};

class BaseChatCompletion {
 public:
  virtual ~BaseChatCompletion() {}

  // General chat method
  virtual void ChatCompletion(
      const HttpRequestPtr &req,
      std::function<void(const HttpResponsePtr &)> &&callback) = 0;
};

class BaseEmbedding {
 public:
  virtual ~BaseEmbedding() {}

  // Implement embedding functionality specific to chat
  virtual void Embedding(
      const HttpRequestPtr &req,
      std::function<void(const HttpResponsePtr &)> &&callback) = 0;

  // The derived class can also override other methods if needed
};

class BaseAudio {
 public:
  virtual ~BaseAudio() {}
  // Transcribes audio into the input language.
  virtual void CreateTranscription(
      const HttpRequestPtr &req,
      std::function<void(const HttpResponsePtr &)> &&callback) = 0;

  //  Translates audio into the input language.
  virtual void CreateTranslation(
      const HttpRequestPtr &req,
      std::function<void(const HttpResponsePtr &)> &&callback) = 0;

  // The derived class can also override other methods if needed
};