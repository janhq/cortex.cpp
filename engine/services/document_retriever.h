#include <json/reader.h>
#include <json/value.h>

#include "extensions/hnswlib/hnswlib.h"
#include "utils/logging_utils.h"

class DocumentRetriever {
 private:
  std::unique_ptr<hnswlib::SpaceInterface<float>> space;
  std::unique_ptr<hnswlib::HierarchicalNSW<float>> index;
  Json::Value docStore;

  bool findDocument(const std::string& doc_id, std::string& content) {
    for (const Json::Value& doc_pair : docStore) {
      // Each doc_pair is an array where first element is ID
      if (doc_pair[0].asString() == doc_id) {
        // Second element is an object containing pageContent
        content = doc_pair[1]["pageContent"].asString();
        return true;
      }
    }
    return false;
  }

  Json::Value LoadJsonFromFile(const std::filesystem::path& path) {
    Json::Value root;
    std::ifstream file(path);
    Json::CharReaderBuilder builder;
    JSONCPP_STRING errs;
    if (!Json::parseFromStream(builder, file, &root, &errs)) {
      throw std::runtime_error("Failed to parse JSON: " + errs);
    }

    return root;
  }

 public:
  DocumentRetriever(const std::filesystem::path& args_path,
                    const std::filesystem::path& docstore_path,
                    const std::filesystem::path& index_path) {
    // load config
    Json::Value args = LoadJsonFromFile(args_path);

    // load docstore
    docStore = LoadJsonFromFile(docstore_path);

    // what is space type
    auto space_type = args["space"].asString();
    auto dimensions = args["numDimensions"].asInt();

    if (space_type == "cosine") {
      space = std::make_unique<hnswlib::InnerProductSpace>(dimensions);
    } else {
      space = std::make_unique<hnswlib::L2Space>(dimensions);
    }

    index = std::make_unique<hnswlib::HierarchicalNSW<float>>(space.get(),
                                                              index_path);
    // can be tweak later on
    // index->setEf(50);
  }

  std::vector<std::pair<std::string, float>> query(
      const std::vector<float>& query_vector, size_t k = 1) {
    std::vector<std::pair<std::string, float>> results;

    // perform kNN search
    std::priority_queue<std::pair<float, size_t>> top_candidates =
        index->searchKnn(query_vector.data(), k);

    while (!top_candidates.empty()) {
      auto current = top_candidates.top();
      auto doc_id = std::to_string(current.second);

      // CLI_LOG("Doc id: " + doc_id);
      std::string content;
      if (findDocument(doc_id, content)) {
        // For debugging
        // CLI_LOG("Found document " << doc_id << " with distance "
        //                           << current.first);
        // CLI_LOG("Doc id: " + doc_id);
        results.emplace_back(content, current.first);
      } else {
        // std::cout << "Warning: Document " << doc_id << " not found in docstore"
        //           << std::endl;
      }
      // if (docStore.isMember(doc_id)) {
      //   results.emplace_back(docStore[doc_id]["pageContent"].asString(),
      //                        current.first);
      // }
      top_candidates.pop();
    }

    return results;
  }
};
