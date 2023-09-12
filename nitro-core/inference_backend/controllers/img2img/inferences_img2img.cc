#include "inferences_img2img.h"
#include "backend_utils.h"
#include "inference_lib.h"
#include <cstdint>

using namespace inference;
using namespace inferences;
using namespace backend_utils;

void img2img::inference(
    const HttpRequestPtr &req,
    std::function<void(const HttpResponsePtr &)> &&callback) {

  // Extract the JSON data from the request body
  auto serve_path = req->getPath();
  auto app_config = drogon::app().getCustomConfig();
  LOG_INFO << serve_path;

  std::string s3_public_endpoint = app_config["s3_public_endpoint"].asString();
  std::string s3_bucket = app_config["s3_bucket"].asString();
  std::string triton_endpoint = app_config["triton_endpoint"].asString();
  // default output shape
  std::vector<int> output_shape = {512, 512, 3};

  std::unique_ptr<inference_tc::InferenceServerGrpcClient> triton_client;
  inference_tc::InferenceServerGrpcClient::Create(&triton_client,
                                                  triton_endpoint);

  std::string response_filename =
      inference_utils::generate_random_string(40) + ".png";
  std::vector<uint8_t> output_data;

  // parser for multipart parser
  MultiPartParser partParser;
  std::string prompt;
  std::string negative_prompt;
  std::string control_net_model;
  int steps = 20;
  int seed = 20000;
  float control_scale;

  partParser.parse(req);
  auto &files = partParser.getFiles();
  const auto &formFields = partParser.getParameters();
  std::string json_data = formFields.at("data");
  std::unique_ptr<Json::Value> jsonBody = std::make_unique<Json::Value>();
  Json::Reader jsonReader;
  jsonReader.parse(json_data, *jsonBody);
  if (jsonBody) {
    prompt = (*jsonBody)["prompt"].asString();
    negative_prompt = (*jsonBody)["neg_prompt"].asString();
    control_net_model = (*jsonBody)["control_net_model"].asString();
    steps = (*jsonBody)["steps"].asInt();
    seed = (*jsonBody)["seed"].asInt64();
    control_scale = (*jsonBody)["control_scale"].asFloat();
    LOG_INFO << prompt << negative_prompt << control_net_model << steps << seed;
  }
  // Assuming you have a pointer to the file data and its length
  const char *imageDataPtr = files[0].fileData();
  size_t imageDataLength = files[0].fileLength();
  // Create a vector to store the image data
  std::vector<uchar> imageData(imageDataPtr, imageDataPtr + imageDataLength);

  // Create an OpenCV Mat object from the image data
  cv::Mat image = cv::imdecode(imageData, cv::IMREAD_COLOR);
  cv::Mat input_image;
  TIME_IT(inference_utils::pad_to_square(image), input_image,
          "Time tiken to pad controlnet image:");

  cv::cvtColor(input_image, input_image, cv::COLOR_BGR2RGB);
  input_image = inference_utils::pad_to_square(input_image);

  TIME_IT(inference::infer_controlnet(triton_client, prompt, negative_prompt,
                           control_net_model, steps, seed, control_scale,
                           input_image),
          output_data, "Time taken to infer from triton for controlnet: ");

  post_process_inference(response_filename, output_data, s3_bucket,
                         s3_public_endpoint, this->s3Client, output_shape);

  Json::Value responseJson;
  responseJson["url"] = s3_public_endpoint + response_filename;

  auto response = drogon::HttpResponse::newHttpJsonResponse(responseJson);
  callback(response);
  triton_client.reset();
  return;
}
