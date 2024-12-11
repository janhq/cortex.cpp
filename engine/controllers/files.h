#pragma once

#include <drogon/HttpController.h>
#include <trantor/utils/Logger.h>
#include <optional>
#include "services/file_service.h"
#include "services/message_service.h"

using namespace drogon;

class Files : public drogon::HttpController<Files, false> {
 public:
  METHOD_LIST_BEGIN
  ADD_METHOD_TO(Files::UploadFile, "/v1/files", Options, Post);

  ADD_METHOD_TO(Files::RetrieveFile, "/v1/files/{file_id}?thread={thread_id}",
                Get);

  ADD_METHOD_TO(
      Files::ListFiles,
      "/v1/files?purpose={purpose}&limit={limit}&order={order}&after={after}",
      Get);

  ADD_METHOD_TO(Files::DeleteFile, "/v1/files/{file_id}", Options, Delete);

  ADD_METHOD_TO(Files::RetrieveFileContent,
                "/v1/files/{file_id}/content?thread={thread_id}", Get);

  METHOD_LIST_END

  explicit Files(std::shared_ptr<FileService> file_service,
                 std::shared_ptr<MessageService> msg_service)
      : file_service_{file_service}, message_service_{msg_service} {}

  void UploadFile(const HttpRequestPtr& req,
                  std::function<void(const HttpResponsePtr&)>&& callback);

  void ListFiles(const HttpRequestPtr& req,
                 std::function<void(const HttpResponsePtr&)>&& callback,
                 std::optional<std::string> purpose,
                 std::optional<std::string> limit,
                 std::optional<std::string> order,
                 std::optional<std::string> after) const;

  void RetrieveFile(const HttpRequestPtr& req,
                    std::function<void(const HttpResponsePtr&)>&& callback,
                    const std::string& file_id,
                    std::optional<std::string> thread_id) const;

  void DeleteFile(const HttpRequestPtr& req,
                  std::function<void(const HttpResponsePtr&)>&& callback,
                  const std::string& file_id);

  void RetrieveFileContent(
      const HttpRequestPtr& req,
      std::function<void(const HttpResponsePtr&)>&& callback,
      const std::string& file_id, std::optional<std::string> thread_id);

 private:
  std::shared_ptr<FileService> file_service_;
  std::shared_ptr<MessageService> message_service_;
};
