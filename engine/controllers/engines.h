#pragma once

#include <drogon/HttpController.h>
#include <drogon/HttpRequest.h>
#include <trantor/utils/Logger.h>
#include <memory>
#include "services/engine_service.h"

using namespace drogon;

class Engines : public drogon::HttpController<Engines, false> {
 public:
  METHOD_LIST_BEGIN

  // install engine
  METHOD_ADD(Engines::InstallEngine, "/{1}/install", Options, Post);
  ADD_METHOD_TO(Engines::InstallEngine, "/v1/engines/{1}/install", Options,
                Post);
  METHOD_ADD(Engines::InstallRemoteEngine, "/engines", Options, Post);
  ADD_METHOD_TO(Engines::InstallRemoteEngine, "/v1/engines", Options, Post);

  // uninstall engine
  METHOD_ADD(Engines::UninstallEngine, "/{1}/install", Options, Delete);
  ADD_METHOD_TO(Engines::UninstallEngine, "/v1/engines/{1}/install", Options,
                Delete);

  // set default engine
  METHOD_ADD(Engines::SetDefaultEngineVariant, "/{1}/default", Options, Post);
  ADD_METHOD_TO(Engines::SetDefaultEngineVariant, "/v1/engines/{1}/default",
                Options, Post);

  // get default engine
  METHOD_ADD(Engines::GetDefaultEngineVariant, "/{1}/default", Get);
  ADD_METHOD_TO(Engines::GetDefaultEngineVariant, "/v1/engines/{1}/default",
                Get);

  // update engine
  METHOD_ADD(Engines::UpdateEngine, "/{1}/update", Options, Post);
  ADD_METHOD_TO(Engines::UpdateEngine, "/v1/engines/{1}/update", Options, Post);

  // load engine
  METHOD_ADD(Engines::LoadEngine, "/{1}/load", Options, Post);
  ADD_METHOD_TO(Engines::LoadEngine, "/v1/engines/{1}/load", Options, Post);

  // unload engine
  METHOD_ADD(Engines::UnloadEngine, "/{1}/load", Options, Delete);
  ADD_METHOD_TO(Engines::UnloadEngine, "/v1/engines/{1}/load", Options, Delete);

  METHOD_ADD(Engines::GetInstalledEngineVariants, "/{1}", Get);
  ADD_METHOD_TO(Engines::GetInstalledEngineVariants, "/v1/engines/{1}", Get);

  METHOD_ADD(Engines::ListEngine, "", Get);
  ADD_METHOD_TO(Engines::ListEngine, "/v1/engines", Get);

  METHOD_ADD(Engines::GetEngineReleases, "/{1}/releases", Get);
  ADD_METHOD_TO(Engines::GetEngineReleases, "/v1/engines/{1}/releases", Get);

  ADD_METHOD_TO(Engines::GetEngineVariants,
                "/v1/engines/{engine}/releases/{version}?show={show}", Get);

  ADD_METHOD_TO(Engines::GetLatestEngineVersion,
                "/v1/engines/{engine}/releases/latest", Get);

  METHOD_LIST_END

  explicit Engines(std::shared_ptr<EngineService> engine_service)
      : engine_service_{engine_service} {}

  void InstallEngine(const HttpRequestPtr& req,
                     std::function<void(const HttpResponsePtr&)>&& callback,
                     const std::string& engine);

  void InstallRemoteEngine(
      const HttpRequestPtr& req,
      std::function<void(const HttpResponsePtr&)>&& callback);

  void UninstallEngine(const HttpRequestPtr& req,
                       std::function<void(const HttpResponsePtr&)>&& callback,
                       const std::string& engine);

  void ListEngine(const HttpRequestPtr& req,
                  std::function<void(const HttpResponsePtr&)>&& callback) const;

  void GetEngineReleases(const HttpRequestPtr& req,
                         std::function<void(const HttpResponsePtr&)>&& callback,
                         const std::string& engine) const;

  void GetEngineVariants(const HttpRequestPtr& req,
                         std::function<void(const HttpResponsePtr&)>&& callback,
                         const std::string& engine, const std::string& version,
                         std::optional<std::string> show) const;

  void GetInstalledEngineVariants(
      const HttpRequestPtr& req,
      std::function<void(const HttpResponsePtr&)>&& callback,
      const std::string& engine) const;

  void GetLatestEngineVersion(
      const HttpRequestPtr& req,
      std::function<void(const HttpResponsePtr&)>&& callback,
      const std::string& engine);

  void UpdateEngine(const HttpRequestPtr& req,
                    std::function<void(const HttpResponsePtr&)>&& callback,
                    const std::string& engine);

  void SetDefaultEngineVariant(
      const HttpRequestPtr& req,
      std::function<void(const HttpResponsePtr&)>&& callback,
      const std::string& engine);

  void GetDefaultEngineVariant(
      const HttpRequestPtr& req,
      std::function<void(const HttpResponsePtr&)>&& callback,
      const std::string& engine) const;

  void LoadEngine(const HttpRequestPtr& req,
                  std::function<void(const HttpResponsePtr&)>&& callback,
                  const std::string& engine);

  void UnloadEngine(const HttpRequestPtr& req,
                    std::function<void(const HttpResponsePtr&)>&& callback,
                    const std::string& engine);

 private:
  std::shared_ptr<EngineService> engine_service_;
};
