#include "inferences_txt2img.h"
#include "backend_utils.h"
#include "inference_lib.h"

using namespace backend_utils;
using namespace inferences;

void txt2img::inference(
    const HttpRequestPtr &req,
    std::function<void(const HttpResponsePtr &)> &&callback) {
  // Extract the JSON data from the request body
  auto serve_path = req->getPath();
  auto app_config = drogon::app().getCustomConfig();
  LOG_INFO << serve_path;

  std::string s3_public_endpoint = app_config["s3_public_endpoint"].asString();
  std::string s3_bucket = app_config["s3_bucket"].asString();
  std::string triton_endpoint = app_config["triton_endpoint"].asString();

  // Default output shape
  std::vector<int> output_shape = {1024, 1024, 3};

  std::unique_ptr<inference_tc::InferenceServerGrpcClient> triton_client;
  inference_tc::InferenceServerGrpcClient::Create(&triton_client,
                                                  triton_endpoint);

  std::string response_filename =
      inference_utils::generate_random_string(40) + ".png";
  std::vector<uint8_t> output_data_sdxl;

  const auto &jsonBody = req->getJsonObject();

  int width = output_shape[0];
  int height = output_shape[1];
  int steps = 20;   // default value
  int seed = 10000; // default value

  std::string prompt = (*jsonBody)["prompt"].asString();
  std::string negative_prompt = (*jsonBody)["neg_prompt"].asString();
  steps = (*jsonBody)["steps"].asInt();
  seed = jsonBody->isMember("seed") ? (*jsonBody)["seed"].asInt() : seed;
  width = jsonBody->isMember("width") ? (*jsonBody)["width"].asInt() : width;
  height =
      jsonBody->isMember("height") ? (*jsonBody)["height"].asInt() : height;

  // Re assign the output shape
  output_shape[0] = width;
  output_shape[1] = height;

  TIME_IT(inference::infer_txt2img(triton_client, prompt, negative_prompt,
                                   width, height, steps, seed, true,
                                   "stable_diffusion_xl"),
          output_data_sdxl, "Time taken to infer from triton for bare sd: ");

  post_process_inference(response_filename, output_data_sdxl, s3_bucket,
                         s3_public_endpoint, this->s3Client, output_shape);

  Json::Value responseJson;
  responseJson["url"] = s3_public_endpoint + response_filename;

  auto response = drogon::HttpResponse::newHttpJsonResponse(responseJson);
  callback(response);
  triton_client.reset();
  return;
}
