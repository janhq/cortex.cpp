#pragma once

#include "validation_macro.h"
#include <drogon/HttpController.h>
#include <inference_lib.h>

using namespace drogon;

// Init schema validation
JSON_VALIDATOR(txt2img,
               "/workspace/workdir/inference_backend/schemas/txt2img.json");

namespace inferences {
class txt2img : public drogon::HttpController<txt2img> {
public:
  txt2img() {
    inference_aws::Client::ClientConfiguration config;
    this->s3Client = inference_aws::S3::S3Client(config);
  };
  METHOD_LIST_BEGIN
  METHOD_ADD(txt2img::inference, "", Post, "txt2img_filter");
  METHOD_LIST_END
  void inference(const HttpRequestPtr &req,
                 std::function<void(const HttpResponsePtr &)> &&callback);

private:
  inference_aws::S3::S3Client s3Client;
};
} // namespace inferences
