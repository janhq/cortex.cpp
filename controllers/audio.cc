#include "audio.h"

#include "utils/nitro_utils.h"
#include "whisper.h"

using namespace v1;

audio::audio() {
  whisper_print_system_info();
};

audio::~audio() {}

std::optional<std::string> audio::ParseModelId(
    const std::shared_ptr<Json::Value>& jsonBody,
    const std::function<void(const HttpResponsePtr&)>& callback) {
  if (!jsonBody->isMember("model_id")) {
    LOG_INFO << "No model_id found in request body";
    Json::Value jsonResp;
    jsonResp["message"] = "No model_id found in request body";
    auto resp = nitro_utils::nitroHttpJsonResponse(jsonResp);
    resp->setStatusCode(k400BadRequest);
    callback(resp);
    return std::nullopt;  // Signal that an error occurred
  }

  return (*jsonBody)["model_id"].asString();
}

void audio::LoadModel(const HttpRequestPtr& req,
                      std::function<void(const HttpResponsePtr&)>&& callback) {
  const auto jsonBody = req->getJsonObject();
  auto optional_model_id = ParseModelId(jsonBody, callback);
  if (!optional_model_id) {
    return;
  }
  std::string model_id = *optional_model_id;

  // Check if model is already loaded
  if (whispers.find(model_id) != whispers.end()) {
    std::string error_msg = "Model " + model_id + " already loaded";
    LOG_INFO << error_msg;
    Json::Value jsonResp;
    jsonResp["message"] = error_msg;
    auto resp = nitro_utils::nitroHttpJsonResponse(jsonResp);
    resp->setStatusCode(k409Conflict);
    callback(resp);
    return;
  }

  // Model not loaded, load it
  // Parse model path from request
  std::string model_path = (*jsonBody)["model_path"].asString();
  if (!is_file_exist(model_path.c_str())) {
    std::string error_msg = "Model " + model_path + " not found";
    LOG_INFO << error_msg;
    Json::Value jsonResp;
    jsonResp["message"] = error_msg;
    auto resp = nitro_utils::nitroHttpJsonResponse(jsonResp);
    resp->setStatusCode(k404NotFound);
    callback(resp);
    return;
  }

  whisper_server_context whisper = whisper_server_context(model_id);
  bool model_loaded = whisper.load_model(model_path);
  // If model failed to load, return a 500 error
  if (!model_loaded) {
    whisper.~whisper_server_context();
    std::string error_msg = "Failed to load model " + model_path;
    LOG_INFO << error_msg;
    Json::Value jsonResp;
    jsonResp["message"] = error_msg;
    auto resp = nitro_utils::nitroHttpJsonResponse(jsonResp);
    resp->setStatusCode(k500InternalServerError);
    callback(resp);
    return;
  }

  // Warm up the model
  // Parse warm up audio path from request
  if (jsonBody->isMember("warm_up_audio_path")) {
    std::string warm_up_msg = "Warming up model " + model_id;
    LOG_INFO << warm_up_msg;
    std::string warm_up_audio_path =
        (*jsonBody)["warm_up_audio_path"].asString();
    // Return 400 error if warm up audio path is not found
    if (!is_file_exist(warm_up_audio_path.c_str())) {
      std::string error_msg =
          "Warm up audio " + warm_up_audio_path +
          " not found, please provide a valid path or don't specify it at all";
      LOG_INFO << error_msg;
      Json::Value jsonResp;
      jsonResp["message"] = error_msg;
      auto resp = nitro_utils::nitroHttpJsonResponse(jsonResp);
      resp->setStatusCode(k400BadRequest);
      callback(resp);
      return;
    } else {
      LOG_INFO << "Warming up model " << model_id << " with audio "
               << warm_up_audio_path << " ...";
      std::string warm_up_result = whisper.inference(warm_up_audio_path, "en",
                                                     "", text_format, 0, false);
      LOG_INFO << "Warm up model " << model_id << " completed";
    }
  } else {
    LOG_INFO << "No warm up audio provided, skipping warm up";
  }

  // Model loaded successfully, add it to the map of loaded models
  // and return a 200 response
  whispers.emplace(model_id, std::move(whisper));
  Json::Value jsonResp;
  std::string success_msg = "Model " + model_id + " loaded successfully";
  jsonResp["message"] = success_msg;
  auto resp = nitro_utils::nitroHttpJsonResponse(jsonResp);
  resp->setStatusCode(k200OK);
  callback(resp);
  return;
}

void audio::UnloadModel(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback) {
  const auto& jsonBody = req->getJsonObject();
  auto optional_model_id = ParseModelId(jsonBody, callback);
  if (!optional_model_id) {
    return;
  }
  std::string model_id = *optional_model_id;

  // If model is not loaded, return a 404 error
  if (whispers.find(model_id) == whispers.end()) {
    std::string error_msg =
        "Model " + model_id +
        " has not been loaded, please load that model into nitro";
    LOG_INFO << error_msg;
    Json::Value jsonResp;
    jsonResp["message"] = error_msg;
    auto resp = nitro_utils::nitroHttpJsonResponse(jsonResp);
    resp->setStatusCode(k404NotFound);
    callback(resp);
    return;
  }

  // Model loaded, unload it
  whispers[model_id].~whisper_server_context();
  whispers.erase(model_id);

  // Return a 200 response
  Json::Value jsonResp;
  std::string success_msg = "Model " + model_id + " unloaded successfully";
  LOG_INFO << success_msg;
  jsonResp["message"] = success_msg;
  auto resp = nitro_utils::nitroHttpJsonResponse(jsonResp);
  resp->setStatusCode(k200OK);
  callback(resp);
  return;
}

void audio::ListModels(const HttpRequestPtr& req,
                       std::function<void(const HttpResponsePtr&)>&& callback) {
  // Return a list of all loaded models
  Json::Value jsonResp;
  Json::Value models;
  for (auto const& model : whispers) {
    models.append(model.first);
  }
  jsonResp["models"] = models;
  auto resp = nitro_utils::nitroHttpJsonResponse(jsonResp);
  resp->setStatusCode(k200OK);
  callback(resp);
  return;
}

void audio::TranscriptionImpl(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback, bool translate) {
  MultiPartParser partParser;
  Json::Value jsonResp;
  if (partParser.parse(req) != 0 || partParser.getFiles().size() != 1) {
    auto resp = HttpResponse::newHttpResponse();
    resp->setBody("Must have exactly one file");
    resp->setStatusCode(k403Forbidden);
    callback(resp);
    return;
  }
  auto& file = partParser.getFiles()[0];
  const auto& formFields = partParser.getParameters();

  // Check if model_id are present in the request. If not, return a 400 error
  if (formFields.find("model_id") == formFields.end()) {
    LOG_INFO << "No model_id found in request body";
    Json::Value jsonResp;
    jsonResp["message"] = "No model_id found in request body";
    auto resp = nitro_utils::nitroHttpJsonResponse(jsonResp);
    resp->setStatusCode(k400BadRequest);
    callback(resp);
    return;
  }

  std::string model_id = formFields.at("model_id");

  // Parse all other optional parameters from the request
  std::string language = formFields.find("language") != formFields.end()
                             ? formFields.at("language")
                             : "en";
  std::string prompt = formFields.find("prompt") != formFields.end()
                           ? formFields.at("prompt")
                           : "";
  std::string response_format =
      formFields.find("response_format") != formFields.end()
          ? formFields.at("response_format")
          : json_format;
  float temperature = formFields.find("temperature") != formFields.end()
                          ? std::stof(formFields.at("temperature"))
                          : 0;

  // Check if model is loaded. If not, return a 404 error
  if (whispers.find(model_id) == whispers.end()) {
    std::string error_msg =
        "Model " + model_id +
        " has not been loaded, please load that model into nitro";
    LOG_INFO << error_msg;
    Json::Value jsonResp;
    jsonResp["message"] = error_msg;
    auto resp = nitro_utils::nitroHttpJsonResponse(jsonResp);
    resp->setStatusCode(k404NotFound);
    callback(resp);
    return;
  }

  // Save input file to temp location
  std::string temp_dir =
      std::filesystem::temp_directory_path().string() + "/" +
      std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(
                         std::chrono::system_clock::now().time_since_epoch())
                         .count());
  // Create the directory
  std::filesystem::create_directory(temp_dir);
  // Save the file to the directory, with its original name
  std::string temp_file_path = temp_dir + "/" + file.getFileName();
  file.saveAs(temp_file_path);

  // Run inference
  std::string result;
  try {
    result =
        whispers[model_id].inference(temp_file_path, language, prompt,
                                     response_format, temperature, translate);
  } catch (const std::exception& e) {
    std::remove(temp_file_path.c_str());
    Json::Value jsonResp;
    jsonResp["message"] = e.what();
    auto resp = nitro_utils::nitroHttpJsonResponse(jsonResp);
    resp->setStatusCode(k500InternalServerError);
    callback(resp);
    return;
  }
  // TODO: Need to remove the entire temp directory, not just the file
  std::remove(temp_file_path.c_str());

  auto resp = nitro_utils::nitroHttpResponse();
  resp->setBody(result);
  resp->setStatusCode(k200OK);
  // Set content type based on response format
  if (response_format == json_format || response_format == vjson_format) {
    resp->addHeader("Content-Type", "application/json");
  } else if (response_format == text_format) {
    resp->addHeader("Content-Type", "text/html");
  } else if (response_format == srt_format) {
    resp->addHeader("Content-Type", "application/x-subrip");
  } else if (response_format == vtt_format) {
    resp->addHeader("Content-Type", "text/vtt");
  }
  callback(resp);
  return;
}

void audio::ModelStatus(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback) {
  auto resp = nitro_utils::nitroHttpResponse();
  resp->setStatusCode(k200OK);
  resp->setContentTypeCode(drogon::CT_APPLICATION_JSON);
  resp->setBody("Unimplemented");
  callback(resp);
}

void audio::CreateTranscription(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback) {
  return TranscriptionImpl(req, std::move(callback), false);
}

void audio::CreateTranslation(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback) {
  return TranscriptionImpl(req, std::move(callback), true);
}