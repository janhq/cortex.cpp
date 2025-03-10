#pragma once

#include "common/message_content.h"
#include "utils/logging_utils.h"

namespace OpenAi {

struct Annotation : JsonSerializable {
  std::string type;

  // The text in the message content that needs to be replaced.
  std::string text;

  uint32_t start_index;

  uint32_t end_index;

  Annotation(const std::string& type, const std::string& text,
             uint32_t start_index, uint32_t end_index)
      : type{type},
        text{text},
        start_index{start_index},
        end_index{end_index} {}

  virtual ~Annotation() = default;
};

// A citation within the message that points to a specific quote from a specific File associated with the assistant or the message. Generated when the assistant uses the "file_search" tool to search files.
struct FileCitationWrapper : Annotation {

  // Always file_citation.
  FileCitationWrapper(const std::string& text, uint32_t start_index,
                      uint32_t end_index)
      : Annotation("file_citation", text, start_index, end_index) {}

  FileCitationWrapper(FileCitationWrapper&&) noexcept = default;

  FileCitationWrapper& operator=(FileCitationWrapper&&) noexcept = default;

  FileCitationWrapper(const FileCitationWrapper&) = delete;

  FileCitationWrapper& operator=(const FileCitationWrapper&) = delete;

  struct FileCitation {
    // The ID of the specific File the citation is from.
    std::string file_id;

    FileCitation() = default;

    FileCitation(FileCitation&&) noexcept = default;

    FileCitation& operator=(FileCitation&&) noexcept = default;

    FileCitation(const FileCitation&) = delete;

    FileCitation& operator=(const FileCitation&) = delete;
  };

  FileCitation file_citation;

  cpp::result<Json::Value, std::string> ToJson() override {
    try {
      Json::Value json;
      json["text"] = text;
      json["type"] = type;
      json["file_citation"]["file_id"] = file_citation.file_id;
      json["start_index"] = start_index;
      json["end_index"] = end_index;
      return json;
    } catch (const std::exception& e) {
      return cpp::fail(std::string("ToJson failed: ") + e.what());
    }
  }
};

// A URL for the file that's generated when the assistant used the code_interpreter tool to generate a file.
struct FilePathWrapper : Annotation {
  // Always file_path.
  FilePathWrapper(const std::string& text, uint32_t start_index,
                  uint32_t end_index)
      : Annotation("file_path", text, start_index, end_index) {}

  FilePathWrapper(FilePathWrapper&&) noexcept = default;

  FilePathWrapper& operator=(FilePathWrapper&&) noexcept = default;

  FilePathWrapper(const FilePathWrapper&) = delete;

  FilePathWrapper& operator=(const FilePathWrapper&) = delete;

  struct FilePath {
    // The ID of the file that was generated.
    std::string file_id;

    FilePath() = default;

    FilePath(FilePath&&) noexcept = default;

    FilePath& operator=(FilePath&&) noexcept = default;

    FilePath(const FilePath&) = delete;

    FilePath& operator=(const FilePath&) = delete;
  };

  FilePath file_path;

  cpp::result<Json::Value, std::string> ToJson() override {
    try {
      Json::Value json;
      json["text"] = text;
      json["type"] = type;
      json["file_path"]["file_id"] = file_path.file_id;
      json["start_index"] = start_index;
      json["end_index"] = end_index;
      return json;
    } catch (const std::exception& e) {
      return cpp::fail(std::string("ToJson failed: ") + e.what());
    }
  }
};

struct Text : JsonSerializable {
  // The data that makes up the text.
  Text() = default;

  Text(Text&&) noexcept = default;

  Text& operator=(Text&&) noexcept = default;

  Text(const Text&) = delete;

  Text& operator=(const Text&) = delete;

  std::string value;

  std::vector<std::unique_ptr<Annotation>> annotations;

  static cpp::result<Text, std::string> FromJson(Json::Value&& json) {
    if (json.empty()) {
      return cpp::fail("Json string is empty");
    }

    try {
      Text text;
      text.value = json["value"].asString();

      // Parse annotations array
      if (json.isMember("annotations") && json["annotations"].isArray()) {
        for (const auto& annotation_json : json["annotations"]) {
          std::string type = annotation_json["type"].asString();
          std::string annotation_text =
              annotation_json["text"].asString();
          uint32_t start_index = annotation_json["start_index"].asUInt();
          uint32_t end_index = annotation_json["end_index"].asUInt();

          if (type == "file_citation") {
            auto citation = std::make_unique<FileCitationWrapper>(
                annotation_text, start_index, end_index);
            citation->file_citation.file_id = std::move(
                annotation_json["file_citation"]["file_id"].asString());
            text.annotations.push_back(std::move(citation));
          } else if (type == "file_path") {
            auto file_path = std::make_unique<FilePathWrapper>(
                annotation_text, start_index, end_index);
            file_path->file_path.file_id =
                std::move(annotation_json["file_path"]["file_id"].asString());
            text.annotations.push_back(std::move(file_path));
          } else {
            CTL_WRN("Unknown annotation type: " + type);
          }
        }
      }

      return text;
    } catch (const std::exception& e) {
      return cpp::fail(std::string("FromJson failed: ") + e.what());
    }
  }

  cpp::result<Json::Value, std::string> ToJson() override {
    try {
      Json::Value json;
      json["value"] = value;
      Json::Value annotations_json_arr{Json::arrayValue};
      for (auto& annotation : annotations) {
        if (auto it = annotation->ToJson(); it.has_value()) {
          annotations_json_arr.append(it.value());
        } else {
          CTL_WRN("Failed to convert annotation to json: " + it.error());
        }
      }
      json["annotations"] = annotations_json_arr;
      return json;
    } catch (const std::exception e) {
      return cpp::fail(std::string("ToJson failed: ") + e.what());
    }
  };
};

// The text content that is part of a message.
struct TextContent : Content {
  // Always text.
  TextContent() : Content("text") {}

  TextContent(TextContent&&) noexcept = default;

  TextContent& operator=(TextContent&&) noexcept = default;

  TextContent(const TextContent&) = delete;

  TextContent& operator=(const TextContent&) = delete;

  Text text;

  ~TextContent() override = default;

  static cpp::result<TextContent, std::string> FromJson(Json::Value&& json) {
    if (json.empty()) {
      return cpp::fail("Json string is empty");
    }

    try {
      TextContent content;
      content.text = std::move(Text::FromJson(std::move(json["text"])).value());
      return content;
    } catch (const std::exception& e) {
      return cpp::fail(std::string("FromJson failed: ") + e.what());
    }
  }

  cpp::result<Json::Value, std::string> ToJson() override {
    try {
      Json::Value json;
      json["type"] = type;
      json["text"] = text.ToJson().value();
      return json;
    } catch (const std::exception& e) {
      return cpp::fail(std::string("ToJson failed: ") + e.what());
    }
  }
};
}  // namespace OpenAi
