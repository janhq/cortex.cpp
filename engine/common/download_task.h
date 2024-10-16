#pragma once

#include <json/json.h>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <sstream>
#include <string>

enum class DownloadType { Model, Engine, Miscellaneous, CudaToolkit, Cortex };
using namespace nlohmann;

struct DownloadItem {
  std::string id;

  std::string downloadUrl;

  /**
   * An absolute path to where the file is located (locally).
   */
  std::filesystem::path localPath;

  std::optional<std::string> checksum;

  std::optional<uint64_t> bytes;

  std::optional<uint64_t> downloadedBytes;

  std::string ToString() const {
    std::ostringstream output;
    output << "DownloadItem{id: " << id << ", downloadUrl: " << downloadUrl
           << ", localContainerPath: " << localPath
           << ", bytes: " << bytes.value_or(0)
           << ", downloadedBytes: " << downloadedBytes.value_or(0)
           << ", checksum: " << checksum.value_or("N/A") << "}";
    return output.str();
  }
};

inline std::string DownloadTypeToString(DownloadType type) {
  switch (type) {
    case DownloadType::Model:
      return "Model";
    case DownloadType::Engine:
      return "Engine";
    case DownloadType::Miscellaneous:
      return "Miscellaneous";
    case DownloadType::CudaToolkit:
      return "CudaToolkit";
    case DownloadType::Cortex:
      return "Cortex";
    default:
      return "Unknown";
  }
}

struct DownloadTask {
  std::string id;

  DownloadType type;

  std::vector<DownloadItem> items;

  std::string ToString() const {
    std::ostringstream output;
    output << "DownloadTask{id: " << id << ", type: " << static_cast<int>(type)
           << ", items: [";
    for (const auto& item : items) {
      output << item.ToString() << ", ";
    }
    output << "]}";
    return output.str();
  }

  Json::Value ToJsonCpp() const {
    Json::Value root;
    root["id"] = id;
    root["type"] = DownloadTypeToString(type);

    Json::Value itemsArray(Json::arrayValue);
    for (const auto& item : items) {
      Json::Value itemObj;
      itemObj["id"] = item.id;
      itemObj["downloadUrl"] = item.downloadUrl;
      itemObj["localPath"] = item.localPath.string();
      itemObj["checksum"] = item.checksum.value_or("N/A");
      itemObj["bytes"] = Json::Value::UInt64(item.bytes.value_or(0));
      itemObj["downloadedBytes"] =
          Json::Value::UInt64(item.downloadedBytes.value_or(0));
      itemsArray.append(itemObj);
    }
    root["items"] = itemsArray;

    return root;
  }

  json ToJson() const {
    json dl_items = json::array();

    for (const auto& item : items) {
      json dl_item{{"id", item.id},
                   {"downloadUrl", item.downloadUrl},
                   {"localPath", item.localPath},
                   {"checksum", item.checksum.value_or("N/A")},
                   {"bytes", item.bytes.value_or(0)},
                   {"downloadedBytes", item.downloadedBytes.value_or(0)}};
      dl_items.push_back(dl_item);
    }

    return json{
        {"id", id}, {"type", DownloadTypeToString(type)}, {"items", dl_items}};
  }
};
