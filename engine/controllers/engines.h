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

  METHOD_ADD(Engines::GetInstalledEngineVariants, "/{1}", Get);
  METHOD_ADD(Engines::InstallEngine, "/{1}?version={2}&variant={3}", Post);
  METHOD_ADD(Engines::UninstallEngine, "/{1}?version={2}&variant={3}", Options,
             Delete);
  METHOD_ADD(Engines::SetDefaultEngineVariant,
             "/{1}/default?version={2}&variant={3}", Post);
  METHOD_ADD(Engines::GetDefaultEngineVariant, "/{1}/default", Get);

  METHOD_ADD(Engines::LoadEngine, "/{1}/load", Post);
  METHOD_ADD(Engines::UnloadEngine, "/{1}/load", Options, Delete);
  METHOD_ADD(Engines::UpdateEngine, "/{1}/update", Post);
  METHOD_ADD(Engines::ListEngine, "", Get);

  METHOD_ADD(Engines::GetEngineVersions, "/{1}/versions", Get);
  METHOD_ADD(Engines::GetEngineVariants, "/{1}/versions/{2}", Get);
  METHOD_ADD(Engines::GetLatestEngineVersion, "/{1}/latest", Get);

  ADD_METHOD_TO(Engines::GetInstalledEngineVariants, "/v1/engines/{1}", Get);
  ADD_METHOD_TO(Engines::InstallEngine,
                "/v1/engines/{1}?version={2}&variant={3}", Post);
  ADD_METHOD_TO(Engines::UninstallEngine,
                "/v1/engines/{1}?version={2}&variant={3}", Options, Delete);
  ADD_METHOD_TO(Engines::SetDefaultEngineVariant,
                "/v1/engines/{1}/default?version={2}&variant={3}", Post);
  ADD_METHOD_TO(Engines::GetDefaultEngineVariant, "/v1/engines/{1}/default",
                Get);

  ADD_METHOD_TO(Engines::LoadEngine, "/v1/engines/{1}/load", Post);
  ADD_METHOD_TO(Engines::UnloadEngine, "/v1/engines/{1}/load", Post);
  ADD_METHOD_TO(Engines::UpdateEngine, "/v1/engines/{1}/update", Post);
  ADD_METHOD_TO(Engines::GetEngineVersions, "/v1/engines/{1}/versions", Get);
  ADD_METHOD_TO(Engines::GetEngineVariants, "/v1/engines/{1}/versions/{2}",
                Get);
  ADD_METHOD_TO(Engines::ListEngine, "/v1/engines", Get);
  METHOD_LIST_END

  explicit Engines(std::shared_ptr<EngineService> engine_service)
      : engine_service_{engine_service} {}

  void ListEngine(const HttpRequestPtr& req,
                  std::function<void(const HttpResponsePtr&)>&& callback) const;

  void UninstallEngine(const HttpRequestPtr& req,
                       std::function<void(const HttpResponsePtr&)>&& callback,
                       const std::string& engine,
                       const std::optional<std::string> version,
                       const std::optional<std::string> variant);

  void GetEngineVersions(const HttpRequestPtr& req,
                         std::function<void(const HttpResponsePtr&)>&& callback,
                         const std::string& engine) const;

  void GetEngineVariants(const HttpRequestPtr& req,
                         std::function<void(const HttpResponsePtr&)>&& callback,
                         const std::string& engine,
                         const std::string& version) const;

  void InstallEngine(const HttpRequestPtr& req,
                     std::function<void(const HttpResponsePtr&)>&& callback,
                     const std::string& engine,
                     const std::optional<std::string> version,
                     const std::optional<std::string> variant_name);

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
      const std::string& engine, const std::string& version,
      const std::string& variant);

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
