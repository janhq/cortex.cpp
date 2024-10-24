#include "engines.h"
#include "services/engine_service.h"
#include "utils/archive_utils.h"
#include "utils/cortex_utils.h"
#include "utils/engine_constants.h"
#include "utils/logging_utils.h"
#include "utils/string_utils.h"

namespace {
// Need to change this after we rename repositories
std::string NormalizeEngine(const std::string& engine) {
  if (engine == kLlamaEngine) {
    return kLlamaRepo;
  } else if (engine == kOnnxEngine) {
    return kOnnxRepo;
  } else if (engine == kTrtLlmEngine) {
    return kTrtLlmRepo;
  }
  return engine;
};
}  // namespace

void Engines::InstallEngine(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback,
    const std::string& engine) {

  if (engine.empty()) {
    Json::Value res;
    res["message"] = "Engine name is required";
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(res);
    resp->setStatusCode(k409Conflict);
    callback(resp);
    LOG_WARN << "No engine field in path param";
    return;
  }

  std::string version = "latest";
  if (auto o = req->getJsonObject(); o) {
    version = (*o).get("version", "latest").asString();
  }

  auto result = engine_service_->InstallEngineAsync(engine, version);
  if (result.has_error()) {
    Json::Value res;
    res["message"] = result.error();
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(res);
    resp->setStatusCode(k400BadRequest);
    callback(resp);
  } else {
    Json::Value res;
    res["message"] = "Engine " + engine + " starts installing!";
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(res);
    resp->setStatusCode(k200OK);
    callback(resp);
  }
}

void Engines::ListEngine(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback) const {
  std::vector<std::string> supported_engines{kLlamaEngine, kOnnxEngine,
                                             kTrtLlmEngine};
  Json::Value ret;
  for (const auto& engine : supported_engines) {
    std::cout << engine << std::endl;

    auto result = engine_service_->GetInstalledEngineVariants(engine);
    if (result.has_error()) {
      continue;
    }
    Json::Value variants(Json::arrayValue);
    for (const auto& variant : result.value()) {
      variants.append(variant.ToJson());
    }
    ret[engine] = variants;
  }

  auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
  resp->setStatusCode(k200OK);
  callback(resp);
}

void Engines::UninstallEngine(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback,
    const std::string& engine, const std::string& version,
    const std::string& variant) {

  auto result =
      engine_service_->UninstallEngineVariant(engine, variant, version);

  Json::Value ret;
  if (result.has_error()) {
    CTL_INF(result.error());
    ret["message"] = result.error();
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
    resp->setStatusCode(k400BadRequest);
    callback(resp);
  } else {
    CTL_INF("Engine uninstalled successfully");
    ret["message"] = "Engine " + engine + " uninstalled successfully!";
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
    resp->setStatusCode(k200OK);
    callback(resp);
  }
}

void Engines::GetEngineVersions(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback,
    const std::string& engine) const {
  if (engine.empty()) {
    Json::Value res;
    res["message"] = "Engine name is required";
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(res);
    resp->setStatusCode(k400BadRequest);
    callback(resp);
    return;
  }

  // TODO: namh support pagination
  auto result = engine_service_->GetEngineReleases(engine);
  if (result.has_error()) {
    Json::Value res;
    res["message"] = "Failed to get engine releases: " + result.error();
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(res);
    resp->setStatusCode(k400BadRequest);
    callback(resp);
    return;
  }

  Json::Value releases(Json::arrayValue);
  for (const auto& release : result.value()) {
    releases.append(release.ToApiJson());
  }
  auto resp = cortex_utils::CreateCortexHttpJsonResponse(releases);
  resp->setStatusCode(k200OK);
  callback(resp);
}

void Engines::GetEngineVariants(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback,
    const std::string& engine, const std::string& version) const {
  if (engine.empty()) {
    Json::Value res;
    res["message"] = "Engine name is required";
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(res);
    resp->setStatusCode(k409Conflict);
    callback(resp);
    LOG_WARN << "No engine field in path param";
    return;
  }

  auto result = engine_service_->GetEngineVariants(engine, version);

  auto normalize_version = string_utils::RemoveSubstring(version, "v");
  Json::Value releases(Json::arrayValue);
  for (const auto& release : result.value()) {
    auto json = release.ToApiJson(NormalizeEngine(engine), normalize_version);
    if (json != std::nullopt) {
      releases.append(json.value());
    }
  }
  auto resp = cortex_utils::CreateCortexHttpJsonResponse(releases);
  resp->setStatusCode(k200OK);
  callback(resp);
}

void Engines::InstallEngineVariant(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback,
    const std::string& engine, const std::string& version,
    const std::string& variant_name) {

  if (version.empty() || variant_name.empty()) {
    Json::Value ret;
    ret["result"] = "Bad Request";
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
    resp->setStatusCode(k400BadRequest);
    callback(resp);
    return;
  }

  auto result =
      engine_service_->InstallEngineAsyncV2(engine, version, variant_name);
  if (result.has_error()) {
    Json::Value res;
    res["message"] = result.error();
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(res);
    resp->setStatusCode(k400BadRequest);
    callback(resp);
  } else {
    Json::Value res;
    res["message"] = "Engine starts installing!";
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(res);
    resp->setStatusCode(k200OK);
    callback(resp);
  }
}

void Engines::GetEnginesInstalledVariants(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback,
    const std::string& engine) const {
  auto result = engine_service_->GetInstalledEngineVariants(engine);
  if (result.has_error()) {
    Json::Value res;
    res["message"] = result.error();
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(res);
    resp->setStatusCode(k400BadRequest);
    callback(resp);
  } else {
    Json::Value releases(Json::arrayValue);
    for (const auto& variant : result.value()) {
      releases.append(variant.ToJson());
    }
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(releases);
    resp->setStatusCode(k200OK);
    callback(resp);
  }
}

void Engines::UpdateEngine(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback,
    const std::string& engine) {}

void Engines::GetLatestEngineVersion(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback,
    const std::string& engine) {
  auto result = engine_service_->GetLatestEngineVersion(engine);
  std::cout << result->ToJson() << std::endl;
  if (result.has_error()) {
    Json::Value res;
    res["message"] = result.error();
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(res);
    resp->setStatusCode(k400BadRequest);
    callback(resp);
  } else {
    auto resp =
        cortex_utils::CreateCortexHttpJsonResponse(result.value().ToApiJson());
    resp->setStatusCode(k200OK);
    callback(resp);
  }
}

void Engines::SetDefaultEngineVariant(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback,
    const std::string& engine, const std::string& version,
    const std::string& variant) {
  auto result =
      engine_service_->SetDefaultEngineVariant(engine, version, variant);
  if (result.has_error()) {
    Json::Value res;
    res["message"] = result.error();
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(res);
    resp->setStatusCode(k400BadRequest);
    callback(resp);
  } else {
    auto resp =
        cortex_utils::CreateCortexHttpJsonResponse(result.value().ToJson());
    resp->setStatusCode(k200OK);
    callback(resp);
  }
}

void Engines::GetDefaultEngineVariant(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback,
    const std::string& engine) const {
  auto result = engine_service_->GetDefaultEngineVariant(engine);
  if (result.has_error()) {
    Json::Value res;
    res["message"] = result.error();
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(res);
    resp->setStatusCode(k400BadRequest);
    callback(resp);
  } else {
    auto resp =
        cortex_utils::CreateCortexHttpJsonResponse(result.value().ToJson());
    resp->setStatusCode(k200OK);
    callback(resp);
  }
}

void Engines::LoadEngine(const HttpRequestPtr& req,
                         std::function<void(const HttpResponsePtr&)>&& callback,
                         const std::string& engine) {
  auto result = engine_service_->LoadEngine(engine);
  if (result.has_error()) {
    Json::Value res;
    res["message"] = result.error();
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(res);
    resp->setStatusCode(k400BadRequest);
    callback(resp);
  } else {
    Json::Value res;
    res["message"] = "Engine " + engine + " loaded successfully!";
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(res);
    resp->setStatusCode(k200OK);
    callback(resp);
  }
}

void Engines::UnloadEngine(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback,
    const std::string& engine) {
  auto result = engine_service_->UnloadEngine(engine);
  if (result.has_error()) {
    Json::Value res;
    res["message"] = result.error();
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(res);
    resp->setStatusCode(k400BadRequest);
    callback(resp);
  } else {
    Json::Value res;
    res["message"] = "Engine " + engine + " unloaded successfully!";
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(res);
    resp->setStatusCode(k200OK);
    callback(resp);
  }
}
