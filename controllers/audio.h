#pragma once

#include <drogon/HttpController.h>
#include <trantor/utils/Logger.h>
#include <optional>
#include "common/base.h"

#define DR_WAV_IMPLEMENTATION
#include "utils/dr_wav.h"

#include "utils/json.hpp"

// Whisper Context
#include "context/whisper_server_context.h"

using json = nlohmann::ordered_json;

using namespace drogon;

namespace v1 {

class audio : public drogon::HttpController<audio>,
              public BaseModel,
              public BaseAudio {
 public:
  audio();
  ~audio();
  METHOD_LIST_BEGIN

  METHOD_ADD(audio::LoadModel, "load_model", Post);
  METHOD_ADD(audio::UnloadModel, "unload_model", Post);
  METHOD_ADD(audio::ListModels, "list_model", Get);
  METHOD_ADD(audio::ModelStatus, "model_status", Get);
  METHOD_ADD(audio::CreateTranscription, "transcriptions", Post);
  METHOD_ADD(audio::CreateTranslation, "translations", Post);

  METHOD_LIST_END
  void LoadModel(
      const HttpRequestPtr& req,
      std::function<void(const HttpResponsePtr&)>&& callback) override;

  void UnloadModel(
      const HttpRequestPtr& req,
      std::function<void(const HttpResponsePtr&)>&& callback) override;

  // TODO: Add to the BaseModel interface
  void ListModels(const HttpRequestPtr& req,
                  std::function<void(const HttpResponsePtr&)>&& callback);

  // TODO: Unimplemented
  void ModelStatus(
      const HttpRequestPtr& req,
      std::function<void(const HttpResponsePtr&)>&& callback) override;

  void CreateTranscription(
      const HttpRequestPtr& req,
      std::function<void(const HttpResponsePtr&)>&& callback) override;

  void CreateTranslation(
      const HttpRequestPtr& req,
      std::function<void(const HttpResponsePtr&)>&& callback) override;

 private:
  std::unordered_map<std::string, whisper_server_context> whispers;

  std::optional<std::string> ParseModelId(
      const std::shared_ptr<Json::Value>& jsonBody,
      const std::function<void(const HttpResponsePtr&)>& callback);

  void TranscriptionImpl(const HttpRequestPtr& req,
                         std::function<void(const HttpResponsePtr&)>&& callback,
                         bool translate);
};
}  // namespace v1