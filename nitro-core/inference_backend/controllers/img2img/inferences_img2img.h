#pragma once

#include "validation_macro.h"
#include <drogon/HttpController.h>
#include <inference_lib.h>

using namespace drogon;

// Init schema validation
// JSON_VALIDATOR(img2img,
//               "/workspace/workdir/inference_backend/schemas/img2img.json");
// TODO: There is yet JSON_VALIDATOR helper for multi-part form data
namespace inferences {
class img2img : public drogon::HttpController<img2img> {
public:
  img2img() {
    inference_aws::Client::ClientConfiguration config;
    this->s3Client = inference_aws::S3::S3Client(config);
  };
  METHOD_LIST_BEGIN
  METHOD_ADD(img2img::inference, "", Post); //, "img2img_filter");
  METHOD_LIST_END
  void inference(const HttpRequestPtr &req,
                 std::function<void(const HttpResponsePtr &)> &&callback);

private:
  inference_aws::S3::S3Client s3Client;
};
} // namespace inferences
