#pragma once

#include "grpc_client.h"
#include <algorithm>
#include <aws/core/Aws.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/model/PutObjectRequest.h>
#include <fstream>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <random>
#include <sentencepiece_processor.h>
#include <string>
#include <trantor/utils/Logger.h>
#include <valijson/adapters/jsoncpp_adapter.hpp>
#include <valijson/schema_parser.hpp>
#include <valijson/validator.hpp>

namespace inference_tc = triton::client;
namespace inference_aws = Aws;
namespace inference_valijson = valijson;
#define FAIL_IF_ERR(X, MSG)                                                    \
  {                                                                            \
    inference_tc::Error err = (X);                                             \
    if (!err.IsOk()) {                                                         \
      std::cerr << "inference_tc::Error: " << (MSG) << ": " << err             \
                << std::endl;                                                  \
      exit(1);                                                                 \
    }                                                                          \
  }

#define TIME_IT_IMPL_NO_RESULT(expression, message)                            \
  do {                                                                         \
    auto start_time = std::chrono::high_resolution_clock::now();               \
    expression;                                                                \
    auto end_time = std::chrono::high_resolution_clock::now();                 \
    auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(  \
                           end_time - start_time)                              \
                           .count();                                           \
    LOG_INFO << message << duration_ms << " milliseconds";                     \
  } while (false)

#define TIME_IT_IMPL_WITH_RESULT(expression, result_variable, message)         \
  do {                                                                         \
    auto start_time = std::chrono::high_resolution_clock::now();               \
    result_variable = expression;                                              \
    auto end_time = std::chrono::high_resolution_clock::now();                 \
    auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(  \
                           end_time - start_time)                              \
                           .count();                                           \
    LOG_INFO << message << duration_ms << " milliseconds";                     \
  } while (false)

#define TIME_IT(...)                                                           \
  GET_MACRO(__VA_ARGS__, TIME_IT_IMPL_WITH_RESULT, TIME_IT_IMPL_NO_RESULT)     \
  (__VA_ARGS__)

#define GET_MACRO(_1, _2, _3, NAME, ...) NAME

// Some fixed value for the endpoint currently
constexpr int batch_size = 1;
constexpr int samples = 1;
constexpr float guidance_scale = 7.5;
constexpr const char scheduler[] = "DPMSolverMultistepScheduler";

namespace inference_utils {

inline void loadSchema(const std::string &schemaFilePath,
                       valijson::Schema &targetSchema) {
  // Load schema document
  Json::Value mySchemaRoot;
  Json::CharReaderBuilder reader;
  Json::Value schemaDocument;

  std::ifstream sd_schemaFile(schemaFilePath, std::ifstream::binary);
  if (!sd_schemaFile) {
    throw std::runtime_error("Failed to open schema file");
  }

  std::string errors;
  if (!Json::parseFromStream(reader, sd_schemaFile, &schemaDocument, &errors)) {
    throw std::runtime_error("Failed to parse sd schema document: " + errors);
  }

  // Populate schema
  valijson::adapters::JsonCppAdapter schemaAdapter(schemaDocument);
  valijson::SchemaParser schemaParser;
  schemaParser.populateSchema(schemaAdapter, targetSchema);
  LOG_INFO << "Your schema from :" << schemaFilePath << "Loaded";
}

inline bool validate_json(const valijson::Schema &schema,
                          const Json::Value &jsonBody,
                          valijson::Validator &validator) {
  valijson::adapters::JsonCppAdapter jsonAdapter(jsonBody);
  valijson::ValidationResults results;

  if (!validator.validate(schema, jsonAdapter, &results)) {
    // Collect validation error messages
    std::string validationErrors;
    for (const auto &error : results) {
      validationErrors += error.description + "\n";
      LOG_INFO << validationErrors;
    }
    LOG_ERROR << "Validation errors: " << validationErrors;
    return false;
  }
  return true;
}

inline int generate_random_positive_int(int upper_limit) {
  std::random_device rd;
  std::mt19937 generator(rd());
  std::uniform_int_distribution<> distribution(1, upper_limit);

  return distribution(generator);
}

inline std::string generate_random_string(std::size_t length) {
  const std::string characters =
      "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

  std::random_device rd;
  std::mt19937 generator(rd());

  std::uniform_int_distribution<> distribution(0, characters.size() - 1);

  std::string random_string(length, '\0');
  std::generate_n(random_string.begin(), length,
                  [&]() { return characters[distribution(generator)]; });

  return random_string;
}

inline cv::Mat pad_to_square(const cv::Mat &image) {
  int width = image.cols;
  int height = image.rows;
  int max_dim = std::max(width, height);

  cv::Mat new_image(
      max_dim, max_dim, image.type(),
      cv::Scalar(255, 255, 255)); // Default background color set to white
  int x_offset = (max_dim - width) / 2;
  int y_offset = (max_dim - height) / 2;

  cv::Rect roi(x_offset, y_offset, width, height);
  image.copyTo(new_image(roi));

  return new_image;
}

} // namespace inference_utils

namespace inference {
class TextCompletions {
private:
  sentencepiece::SentencePieceProcessor processor;
  std::string url;
  // Assistant cost 10 position in the tokenizer (strangely enough)
  int prev_pos = 10;
  int prev_size = 0;

  void InitializeClient() {
    FAIL_IF_ERR(inference_tc::InferenceServerGrpcClient::Create(&triton_client,
                                                                url, false),
                "unable to create grpc client");
    const auto status =
        processor.Load("/workspace/workdir/models/tokenizer.model");
  }

public:
  std::unique_ptr<inference_tc::InferenceServerGrpcClient> triton_client;
  std::mutex mutex_;
  std::condition_variable cv_;
  std::shared_ptr<std::vector<std::string>> result_list =
      std::make_shared<std::vector<std::string>>();
  TextCompletions(std::string llm_host) {
    this->url = llm_host;
    InitializeClient();
  }

  void GetCompletions(std::string model_name, std::string text_prompt,
                      int max_tokens, float top_p, float temperature,
                      uint64_t sequence_id, bool sequence_start,
                      int stream_timeout) {

    inference_tc::InferOptions options(model_name);
    // Input value here id START_in END_in and request id
    options.sequence_id_ = sequence_id;
    options.sequence_start_ = sequence_start;
    options.sequence_end_ = !sequence_start;
    options.request_id_ = std::to_string(options.sequence_id_) + "_" +
                          inference_utils::generate_random_string(
                              5); // replace with format in future c++ standard

    std::vector<int> input_ids = processor.EncodeAsIds(
        text_prompt); // right here it int but it's actually
    // This will be input
    int input_len = input_ids.size();
    std::vector<uint32_t> request_output_len_in_value{
        static_cast<uint32_t>(max_tokens)};
    // Input value here
    std::vector<float> runtime_top_p_in_value{top_p};
    // Input value here
    std::vector<uint32_t> session_len_in_value{
        static_cast<uint32_t>(max_tokens + input_len)};
    // Input value here
    std::vector<int32_t> START_in_value{sequence_start ? 1 : 0};
    // Input value here
    std::vector<float> temperature_in_value{temperature};
    // Input value here
    std::vector<int32_t> END_in_value{!sequence_start ? 1 : 0};

    // Input value here
    std::vector<uint64_t> CORRID_in_value{sequence_id};
    // Input value here
    std::vector<uint64_t> random_seed_in_value{
        static_cast<uint64_t>(inference_utils::generate_random_positive_int(
            10000))}; // replace with real random here

    FAIL_IF_ERR(
        triton_client->StartStream(
            [this, input_len, max_tokens,
             sequence_id](inference_tc::InferResult *result) {
              std::unique_lock<std::mutex> ulk(mutex_);

              auto err = result->RequestStatus();
              if (!err.IsOk()) {
                result_list->push_back("[DONE]");
                cv_.notify_all();
                LOG_INFO << "Strange Bug Happen";
                return;
              }
              ulk.unlock();
              const uint8_t *output_buffer;
              size_t output_byte_size;
              FAIL_IF_ERR(result->RawData("output_ids", &output_buffer,
                                          &output_byte_size),
                          "unable to get raw data for 'output_ids'");

              size_t output_element_count = output_byte_size / sizeof(int);
              std::vector<int> output_data(output_element_count);
              memcpy(output_data.data(), output_buffer, output_byte_size);
              output_data.erase(
                  std::remove(output_data.begin(), output_data.end(), 0),
                  output_data.end());

              // Erase the beginning part
              output_data.erase(output_data.begin(),
                                output_data.begin() + input_len);

              int lastValue = output_data.back();
              int cur_size = output_data.size();

              ulk.lock();

              // Because we already remove the input len, there is no need to
              // add back input len We need to add +2 because assistant took 2
              // tokens
              if ((lastValue == 2 || cur_size == max_tokens + 1) &&
                  cur_size == prev_size) {

                result_list->push_back("[DONE]");

                cv_.notify_all();
                return;
              } else if (lastValue == 2 ||
                         (cur_size == input_len + max_tokens + 1)) {
                prev_size = cur_size;

                std::string text_back = processor.DecodeIds(output_data);
                if (prev_pos <
                    text_back.size() + 1) { ////// TO DO : Check ambigous
                                            /// behaviour when end pre-mature
                  result_list->push_back(text_back.substr(prev_pos) + "|" +
                                         "[PRE]");
                  prev_pos = text_back.size();
                  cv_.notify_all();
                }
                return;
              }

              prev_size = cur_size;

              std::string text_back = processor.DecodeIds(output_data);
              if (prev_pos < text_back.size()) {
                result_list->push_back(text_back.substr(prev_pos));
                prev_pos = text_back.size();
                cv_.notify_all();
              }
              ulk.unlock();
              return;
            },
            false, 1e6 * stream_timeout),
        "unable to establish a streaming connection to server");

    inference_tc::InferInput *input_ids_in;
    inference_tc::InferInput *input_lengths_in;
    inference_tc::InferInput *request_output_len_in;
    inference_tc::InferInput *runtime_top_p_in;
    inference_tc::InferInput *temperature_in;
    inference_tc::InferInput *repetition_penalty_in;
    inference_tc::InferInput *step_in;
    inference_tc::InferInput *session_len_in;
    inference_tc::InferInput *START_in;
    inference_tc::InferInput *END_in;
    inference_tc::InferInput *CORRID_in;
    inference_tc::InferInput *STOP_in;
    inference_tc::InferInput *random_seed_in;

    std::vector<int64_t> input_ids_in_shape{
        1, static_cast<int64_t>(input_ids.size())};
    std::vector<int64_t> input_lengths_in_shape{1, 1};
    std::vector<int64_t> request_output_len_in_shape{1, 1};
    std::vector<int64_t> runtime_top_p_in_shape{1, 1};
    std::vector<int64_t> temperature_in_shape{1, 1};
    std::vector<int64_t> repetition_penalty_in_shape{1, 1};
    std::vector<int64_t> step_in_shape{1, 1};
    std::vector<int64_t> session_len_in_shape{1, 1};
    std::vector<int64_t> START_in_shape{1, 1};
    std::vector<int64_t> END_in_shape{1, 1};
    std::vector<int64_t> CORRID_in_shape{1, 1};
    std::vector<int64_t> STOP_in_shape{1, 1};
    std::vector<int64_t> random_seed_in_shape{1, 1};

    FAIL_IF_ERR(inference_tc::InferInput::Create(&input_ids_in, "input_ids",
                                                 input_ids_in_shape, "UINT32"),
                "unable to create 'input_ids_in'");
    std::shared_ptr<inference_tc::InferInput> input_ids_in_ptr;
    input_ids_in_ptr.reset(input_ids_in);

    FAIL_IF_ERR(
        inference_tc::InferInput::Create(&input_lengths_in, "input_lengths",
                                         input_lengths_in_shape, "UINT32"),
        "unable to create 'input_ids_in'");
    std::shared_ptr<inference_tc::InferInput> input_lengths_in_ptr;
    input_lengths_in_ptr.reset(input_lengths_in);

    FAIL_IF_ERR(inference_tc::InferInput::Create(
                    &request_output_len_in, "request_output_len",
                    request_output_len_in_shape, "UINT32"),
                "unable to create 'input_ids_in'");
    std::shared_ptr<inference_tc::InferInput> request_output_len_in_ptr;
    request_output_len_in_ptr.reset(request_output_len_in);

    FAIL_IF_ERR(
        inference_tc::InferInput::Create(&runtime_top_p_in, "runtime_top_p",
                                         runtime_top_p_in_shape, "FP32"),
        "unable to create 'runtime_top_p_in'");
    std::shared_ptr<inference_tc::InferInput> runtime_top_p_in_ptr;
    runtime_top_p_in_ptr.reset(runtime_top_p_in);

    FAIL_IF_ERR(inference_tc::InferInput::Create(&temperature_in, "temperature",
                                                 temperature_in_shape, "FP32"),
                "unable to create 'temperature_in'");
    std::shared_ptr<inference_tc::InferInput> temperature_in_ptr;
    temperature_in_ptr.reset(temperature_in);

    FAIL_IF_ERR(inference_tc::InferInput::Create(
                    &repetition_penalty_in, "repetition_penalty",
                    repetition_penalty_in_shape, "FP32"),
                "unable to create 'repetition_penalty_in'");
    std::shared_ptr<inference_tc::InferInput> repetition_penalty_in_ptr;
    repetition_penalty_in_ptr.reset(repetition_penalty_in);

    FAIL_IF_ERR(inference_tc::InferInput::Create(&step_in, "step",
                                                 step_in_shape, "INT32"),
                "unable to create 'step_in'");
    std::shared_ptr<inference_tc::InferInput> step_in_ptr;
    step_in_ptr.reset(step_in);

    FAIL_IF_ERR(inference_tc::InferInput::Create(&session_len_in, "session_len",
                                                 session_len_in_shape,
                                                 "UINT32"),
                "unable to create 'session_len_in'");
    std::shared_ptr<inference_tc::InferInput> session_len_in_ptr;
    session_len_in_ptr.reset(session_len_in);

    FAIL_IF_ERR(inference_tc::InferInput::Create(&START_in, "START",
                                                 START_in_shape, "INT32"),
                "unable to create 'START_in'");
    std::shared_ptr<inference_tc::InferInput> START_in_ptr;
    START_in_ptr.reset(START_in);

    FAIL_IF_ERR(
        inference_tc::InferInput::Create(&END_in, "END", END_in_shape, "INT32"),
        "unable to create 'END_in'");
    std::shared_ptr<inference_tc::InferInput> END_in_ptr;
    END_in_ptr.reset(END_in);

    FAIL_IF_ERR(inference_tc::InferInput::Create(&CORRID_in, "CORRID",
                                                 CORRID_in_shape, "UINT64"),
                "unable to create 'CORRID_in'");
    std::shared_ptr<inference_tc::InferInput> CORRID_in_ptr;
    CORRID_in_ptr.reset(CORRID_in);

    FAIL_IF_ERR(inference_tc::InferInput::Create(&STOP_in, "STOP",
                                                 STOP_in_shape, "INT32"),
                "unable to create 'STOP_in'");
    std::shared_ptr<inference_tc::InferInput> STOP_in_ptr;
    STOP_in_ptr.reset(STOP_in);

    FAIL_IF_ERR(inference_tc::InferInput::Create(&random_seed_in, "random_seed",
                                                 random_seed_in_shape,
                                                 "UINT64"),
                "unable to create 'random_seed_in'");
    std::shared_ptr<inference_tc::InferInput> random_seed_in_ptr;
    random_seed_in_ptr.reset(random_seed_in);

    FAIL_IF_ERR(input_ids_in_ptr->AppendRaw(
                    reinterpret_cast<uint8_t *>(input_ids.data()),
                    input_ids.size() * sizeof(int)),
                "unable to set data for 'input_ids_in'");
    FAIL_IF_ERR(
        request_output_len_in_ptr->AppendRaw(
            reinterpret_cast<uint8_t *>(request_output_len_in_value.data()),
            request_output_len_in_value.size() * sizeof(int32_t)),
        "unable to set data for 'request_output_len_in'");

    std::vector<uint32_t> input_lengths_in_value{
        static_cast<uint32_t>(input_ids.size())};
    FAIL_IF_ERR(input_lengths_in_ptr->AppendRaw(
                    reinterpret_cast<uint8_t *>(input_lengths_in_value.data()),
                    input_lengths_in_value.size() * sizeof(int32_t)),
                "unable to set data for 'input_lengths_in'");
    FAIL_IF_ERR(runtime_top_p_in_ptr->AppendRaw(
                    reinterpret_cast<uint8_t *>(runtime_top_p_in_value.data()),
                    runtime_top_p_in_value.size() * sizeof(float)),
                "unable to set data for 'runtime_top_p_in'");

    FAIL_IF_ERR(temperature_in_ptr->AppendRaw(
                    reinterpret_cast<uint8_t *>(temperature_in_value.data()),
                    temperature_in_value.size() * sizeof(float)),
                "unable to set data for 'temperature_in'");

    std::vector<float> repetition_penalty_in_value{1.0};
    FAIL_IF_ERR(
        repetition_penalty_in_ptr->AppendRaw(
            reinterpret_cast<uint8_t *>(repetition_penalty_in_value.data()),
            repetition_penalty_in_value.size() * sizeof(float)),
        "unable to set data for 'repetition_penalty_in'");

    std::vector<int32_t> step_in_value{0};
    FAIL_IF_ERR(step_in_ptr->AppendRaw(
                    reinterpret_cast<uint8_t *>(step_in_value.data()),
                    step_in_value.size() * sizeof(int32_t)),
                "unable to set data for 'step_in'");

    FAIL_IF_ERR(session_len_in_ptr->AppendRaw(
                    reinterpret_cast<uint8_t *>(session_len_in_value.data()),
                    session_len_in_value.size() * sizeof(uint32_t)),
                "unable to set data for 'session_len_in'");
    FAIL_IF_ERR(START_in_ptr->AppendRaw(
                    reinterpret_cast<uint8_t *>(START_in_value.data()),
                    START_in_value.size() * sizeof(int32_t)),
                "unable to set data for 'START_in'");

    FAIL_IF_ERR(
        END_in_ptr->AppendRaw(reinterpret_cast<uint8_t *>(END_in_value.data()),
                              END_in_value.size() * sizeof(int32_t)),
        "unable to set data for 'END_in'");

    FAIL_IF_ERR(CORRID_in_ptr->AppendRaw(
                    reinterpret_cast<uint8_t *>(CORRID_in_value.data()),
                    CORRID_in_value.size() * sizeof(uint64_t)),
                "unable to set data for 'CORRID_in'");

    std::vector<int32_t> STOP_in_value{0};
    FAIL_IF_ERR(STOP_in_ptr->AppendRaw(
                    reinterpret_cast<uint8_t *>(STOP_in_value.data()),
                    STOP_in_value.size() * sizeof(int32_t)),
                "unable to set data for 'STOP_in'");

    FAIL_IF_ERR(random_seed_in_ptr->AppendRaw(
                    reinterpret_cast<uint8_t *>(random_seed_in_value.data()),
                    random_seed_in_value.size() * sizeof(uint64_t)),
                "unable to set data for 'random_seed_in'");

    std::vector<inference_tc::InferInput *> inputs = {
        input_ids_in_ptr.get(),
        input_lengths_in_ptr.get(),
        request_output_len_in_ptr.get(),
        runtime_top_p_in_ptr.get(),
        temperature_in_ptr.get(),
        repetition_penalty_in_ptr.get(),
        step_in_ptr.get(),
        session_len_in_ptr.get(),
        START_in_ptr.get(),
        END_in_ptr.get(),
        CORRID_in_ptr.get(),
        STOP_in_ptr.get(),
        random_seed_in_ptr.get()};

    FAIL_IF_ERR(triton_client->AsyncStreamInfer(options, inputs),
                "unable to run model");
  }
};

template <typename T>
inline std::vector<uchar> process_image(std::vector<T> &data,
                                        const std::vector<int> &shape) {
  static_assert(std::is_same_v<T, float> || std::is_same_v<T, uint8_t>,
                "Unsupported data type");

  // Deduce the OpenCV type from the data type T
  int type;
  if constexpr (std::is_same_v<T, float>) {
    type = CV_32FC3;
  } else if constexpr (std::is_same_v<T, uint8_t>) {
    type = CV_8UC3;
  }

  // Create an empty Mat and reshape the vector to image
  cv::Mat image(shape[0], shape[1], type, data.data());

  if constexpr (std::is_same_v<T, float>) {
    // Normalize the image (scale to 0-255 and convert to uchar) if float
    image.convertTo(image, CV_8UC3, 255.0);
  }

  // OpenCV stores image in BGR format by default, so convert it toRGB
  cv::cvtColor(image, image, cv::COLOR_BGR2RGB);

  // Encode the image to png and store in buffer
  std::vector<uchar> buffer;
  cv::imencode(".png", image, buffer);
  return buffer;
}

inline std::vector<uint8_t> infer_txt2img(
    std::unique_ptr<inference_tc::InferenceServerGrpcClient> &triton_client,
    std::string prompt, std::string negative_prompt, int width, int height,
    int steps, int seed, bool compel, std::string model_name) {
  // Input placeholder
  inference_tc::InferInput *prompt_in;
  inference_tc::InferInput *negative_prompt_in;
  inference_tc::InferInput *width_in;
  inference_tc::InferInput *height_in;
  inference_tc::InferInput *samples_in;
  inference_tc::InferInput *scheduler_in;
  inference_tc::InferInput *steps_in;
  inference_tc::InferInput *guidance_scale_in;
  inference_tc::InferInput *seed_in;
  inference_tc::InferInput *compel_in;

  FAIL_IF_ERR(inference_tc::InferInput::Create(&prompt_in, "PROMPT",
                                               {batch_size}, "BYTES"),
              "unable to create PROMPT input");
  std::shared_ptr<inference_tc::InferInput> prompt_in_ptr;
  prompt_in_ptr.reset(prompt_in);

  FAIL_IF_ERR(inference_tc::InferInput::Create(&negative_prompt_in,
                                               "NEGATIVE_PROMPT", {batch_size},
                                               "BYTES"),
              "unable to create NEGATIVE_PROMPT input");
  std::shared_ptr<inference_tc::InferInput> negative_prompt_in_ptr;
  negative_prompt_in_ptr.reset(negative_prompt_in);

  FAIL_IF_ERR(inference_tc::InferInput::Create(&width_in, "WIDTH", {batch_size},
                                               "INT32"),
              "unable to create WIDTH input");
  std::shared_ptr<inference_tc::InferInput> width_in_ptr;
  width_in_ptr.reset(width_in);

  FAIL_IF_ERR(inference_tc::InferInput::Create(&height_in, "HEIGHT",
                                               {batch_size}, "INT32"),
              "unable to create HEIGHT input");
  std::shared_ptr<inference_tc::InferInput> height_in_ptr;
  height_in_ptr.reset(height_in);

  FAIL_IF_ERR(inference_tc::InferInput::Create(&samples_in, "SAMPLES",
                                               {batch_size}, "INT32"),
              "unable to create SAMPLES input");
  std::shared_ptr<inference_tc::InferInput> samples_in_ptr;
  samples_in_ptr.reset(samples_in);

  FAIL_IF_ERR(inference_tc::InferInput::Create(&scheduler_in, "SCHEDULER",
                                               {batch_size}, "BYTES"),
              "unable to create SCHEDULER input");
  std::shared_ptr<inference_tc::InferInput> scheduler_in_ptr;
  scheduler_in_ptr.reset(scheduler_in);

  FAIL_IF_ERR(inference_tc::InferInput::Create(&steps_in, "STEPS", {batch_size},
                                               "INT32"),
              "unable to create STEPS input");
  std::shared_ptr<inference_tc::InferInput> steps_in_ptr;
  steps_in_ptr.reset(steps_in);

  FAIL_IF_ERR(inference_tc::InferInput::Create(
                  &guidance_scale_in, "GUIDANCE_SCALE", {batch_size}, "FP32"),
              "unable to create GUIDANCE_SCALE input");
  std::shared_ptr<inference_tc::InferInput> guidance_scale_in_ptr;
  guidance_scale_in_ptr.reset(guidance_scale_in);

  FAIL_IF_ERR(
      inference_tc::InferInput::Create(&seed_in, "SEED", {batch_size}, "INT64"),
      "unable to create SEED input");
  std::shared_ptr<inference_tc::InferInput> seed_in_ptr;
  seed_in_ptr.reset(seed_in);

  FAIL_IF_ERR(inference_tc::InferInput::Create(&compel_in, "USE_COMPEL",
                                               {batch_size}, "BOOL"),
              "unable to create USE_COMPEL input");
  std::shared_ptr<inference_tc::InferInput> compel_in_ptr;
  compel_in_ptr.reset(compel_in);

  // Setting inputs
  std::vector<std::string> prompts(batch_size, prompt);
  std::vector<std::string> negative_prompts(batch_size, negative_prompt);
  std::vector<int32_t> widths(batch_size, width);
  std::vector<int32_t> heights(batch_size, height);
  std::vector<int32_t> samples_arr(batch_size, samples);
  std::vector<std::string> schedulers(batch_size, scheduler);
  std::vector<int32_t> steps_arr(batch_size, steps);
  std::vector<float> guidance_scales(batch_size, guidance_scale);
  std::vector<int64_t> seeds(batch_size, seed);
  int use_compel = compel ? 1 : 0;
  std::vector<int> compels = {use_compel};

  // Add user input into client input

  FAIL_IF_ERR(prompt_in_ptr->AppendFromString(prompts),
              "unable to set data for PROMPTS");

  FAIL_IF_ERR(negative_prompt_in_ptr->AppendFromString(negative_prompts),
              "unable to set data for NEGATIVE_PROMPTS");

  FAIL_IF_ERR(
      width_in_ptr->AppendRaw(reinterpret_cast<const uint8_t *>(widths.data()),
                              widths.size() * sizeof(int32_t)),
      "unable to set data for SAMPLES");

  FAIL_IF_ERR(height_in_ptr->AppendRaw(
                  reinterpret_cast<const uint8_t *>(heights.data()),
                  heights.size() * sizeof(int32_t)),
              "unable to set data for SAMPLES");

  FAIL_IF_ERR(samples_in_ptr->AppendRaw(
                  reinterpret_cast<const uint8_t *>(samples_arr.data()),
                  samples_arr.size() * sizeof(int32_t)),
              "unable to set data for SAMPLES");

  FAIL_IF_ERR(scheduler_in_ptr->AppendFromString(schedulers),
              "unable to set data for SCHEDULERS");

  FAIL_IF_ERR(steps_in_ptr->AppendRaw(
                  reinterpret_cast<const uint8_t *>(steps_arr.data()),
                  steps_arr.size() * sizeof(int32_t)),
              "unable to set data for STEPS");

  FAIL_IF_ERR(guidance_scale_in_ptr->AppendRaw(
                  reinterpret_cast<const uint8_t *>(guidance_scales.data()),
                  guidance_scales.size() * sizeof(float)),
              "unable to set data for GUIDANCE_SCALES");

  FAIL_IF_ERR(
      seed_in_ptr->AppendRaw(reinterpret_cast<const uint8_t *>(seeds.data()),
                             seeds.size() * sizeof(int64_t)),
      "unable to set data for SEEDS");

  FAIL_IF_ERR(compel_in_ptr->AppendRaw(
                  reinterpret_cast<const uint8_t *>(compels.data()),
                  compels.size() * sizeof(int)),
              "unable to set data for SAMPLES");

  // Create Requested Outputs
  inference_tc::InferRequestedOutput *images;
  FAIL_IF_ERR(inference_tc::InferRequestedOutput::Create(&images, "IMAGES"),
              "unable to set output data");
  std::shared_ptr<inference_tc::InferRequestedOutput> images_ptr;
  images_ptr.reset(images);

  // Set inputs and outputs
  auto inputs = {prompt_in_ptr.get(),         negative_prompt_in_ptr.get(),
                 width_in_ptr.get(),          height_in_ptr.get(),
                 scheduler_in_ptr.get(),      steps_in_ptr.get(),
                 guidance_scale_in_ptr.get(), seed_in_ptr.get(),
                 compel_in_ptr.get()};

  std::vector<const inference_tc::InferRequestedOutput *> outputs = {
      images_ptr.get()};

  inference_tc::InferOptions options(model_name);
  // options.model_name_ = "stable_diffusion_xl";
  options.model_version_ = "1";

  // Set results and query the model
  inference_tc::InferResult *results;
  FAIL_IF_ERR(triton_client->Infer(&results, options, inputs, outputs),
              "unable to run model");
  std::shared_ptr<inference_tc::InferResult> results_ptr;
  results_ptr.reset(results);

  const uint8_t *output_buffer;
  size_t output_byte_size;
  FAIL_IF_ERR(results_ptr->RawData("IMAGES", &output_buffer, &output_byte_size),
              "unable to get raw data for 'IMAGES'");

  size_t output_element_count = output_byte_size / sizeof(uint8_t);
  std::vector<uint8_t> output_data(output_element_count);
  memcpy(output_data.data(), output_buffer, output_byte_size);
  return output_data;
}
inline std::vector<uint8_t> infer_controlnet(
    std::unique_ptr<inference_tc::InferenceServerGrpcClient> &triton_client,
    std::string prompt, std::string negative_prompt,
    std::string control_net_model, int steps, int seed, float control_scale,
    cv::Mat input_image) {

  std::vector<uint8_t> control_image_input_data;
  if (input_image.isContinuous()) {
    control_image_input_data.assign(
        input_image.data,
        input_image.data + input_image.total() * input_image.elemSize());
  } else {
    LOG_ERROR << "Error: Image data is not continuous.";
  }

  std::vector<int64_t> control_image_shape = {
      1, input_image.rows, input_image.cols, input_image.channels()};

  // Input placeholder
  inference_tc::InferInput *unet_model_in;
  inference_tc::InferInput *prompt_in;
  inference_tc::InferInput *negative_prompt_in;
  inference_tc::InferInput *samples_in;
  inference_tc::InferInput *scheduler_in;
  inference_tc::InferInput *steps_in;
  inference_tc::InferInput *guidance_scale_in;
  inference_tc::InferInput *seed_in;
  inference_tc::InferInput *control_images_in;
  inference_tc::InferInput *control_scales_in;

  FAIL_IF_ERR(inference_tc::InferInput::Create(
                  &unet_model_in, "UNET_MODEL_NAME", {batch_size}, "BYTES"),
              "unable to create UNET_MODEL_NAME input");
  std::shared_ptr<inference_tc::InferInput> unet_model_in_ptr;
  unet_model_in_ptr.reset(unet_model_in);

  FAIL_IF_ERR(inference_tc::InferInput::Create(&prompt_in, "PROMPT",
                                               {batch_size}, "BYTES"),
              "unable to create PROMPT input");
  std::shared_ptr<inference_tc::InferInput> prompt_in_ptr;
  prompt_in_ptr.reset(prompt_in);

  FAIL_IF_ERR(inference_tc::InferInput::Create(&negative_prompt_in,
                                               "NEGATIVE_PROMPT", {batch_size},
                                               "BYTES"),
              "unable to create NEGATIVE_PROMPT input");
  std::shared_ptr<inference_tc::InferInput> negative_prompt_in_ptr;
  negative_prompt_in_ptr.reset(negative_prompt_in);

  FAIL_IF_ERR(inference_tc::InferInput::Create(&samples_in, "SAMPLES",
                                               {batch_size}, "INT32"),
              "unable to create SAMPLES input");
  std::shared_ptr<inference_tc::InferInput> samples_in_ptr;
  samples_in_ptr.reset(samples_in);

  FAIL_IF_ERR(inference_tc::InferInput::Create(&scheduler_in, "SCHEDULER",
                                               {batch_size}, "BYTES"),
              "unable to create SCHEDULER input");
  std::shared_ptr<inference_tc::InferInput> scheduler_in_ptr;
  scheduler_in_ptr.reset(scheduler_in);

  FAIL_IF_ERR(inference_tc::InferInput::Create(&steps_in, "STEPS", {batch_size},
                                               "INT32"),
              "unable to create STEPS input");
  std::shared_ptr<inference_tc::InferInput> steps_in_ptr;
  steps_in_ptr.reset(steps_in);

  FAIL_IF_ERR(inference_tc::InferInput::Create(
                  &guidance_scale_in, "GUIDANCE_SCALE", {batch_size}, "FP32"),
              "unable to create GUIDANCE_SCALE input");
  std::shared_ptr<inference_tc::InferInput> guidance_scale_in_ptr;
  guidance_scale_in_ptr.reset(guidance_scale_in);

  FAIL_IF_ERR(
      inference_tc::InferInput::Create(&seed_in, "SEED", {batch_size}, "INT64"),
      "unable to create SEED input");
  std::shared_ptr<inference_tc::InferInput> seed_in_ptr;
  seed_in_ptr.reset(seed_in);

  FAIL_IF_ERR(inference_tc::InferInput::Create(&control_images_in,
                                               "CONTROLNET_IMAGES",
                                               control_image_shape, "UINT8"),
              "unable to create CONTROLNET_IMAGES input");
  std::shared_ptr<inference_tc::InferInput> control_images_in_ptr;
  control_images_in_ptr.reset(control_images_in);

  FAIL_IF_ERR(inference_tc::InferInput::Create(&control_scales_in,
                                               "CONTROLNET_SCALES",
                                               {batch_size}, "FP32"),
              "unable to create CONTROLNET_SCALES input");
  std::shared_ptr<inference_tc::InferInput> controlnet_scales_in_ptr;
  controlnet_scales_in_ptr.reset(control_scales_in);

  // Setting inputs
  std::vector<std::string> unet_models(batch_size, control_net_model);
  std::vector<std::string> prompts(batch_size, prompt);
  std::vector<std::string> negative_prompts(batch_size, negative_prompt);
  std::vector<int32_t> samples_arr(batch_size, samples);
  std::vector<std::string> schedulers(batch_size, scheduler);
  std::vector<int32_t> steps_arr(batch_size, steps);
  std::vector<float> guidance_scales(batch_size, guidance_scale);
  std::vector<int64_t> seeds(batch_size, seed);
  std::vector<float> controlnet_scales(batch_size, control_scale);
  // printImageData(input_image_data);
  //  Add user input into client input
  FAIL_IF_ERR(unet_model_in_ptr->AppendFromString(unet_models),
              "unable to set data for UNET_MODEL_NAME");

  FAIL_IF_ERR(prompt_in_ptr->AppendFromString(prompts),
              "unable to set data for PROMPTS");

  FAIL_IF_ERR(negative_prompt_in_ptr->AppendFromString(negative_prompts),
              "unable to set data for NEGATIVE_PROMPTS");

  FAIL_IF_ERR(samples_in_ptr->AppendRaw(
                  reinterpret_cast<const uint8_t *>(samples_arr.data()),
                  samples_arr.size() * sizeof(int32_t)),
              "unable to set data for SAMPLES");

  FAIL_IF_ERR(scheduler_in_ptr->AppendFromString(schedulers),
              "unable to set data for SCHEDULERS");

  FAIL_IF_ERR(steps_in_ptr->AppendRaw(
                  reinterpret_cast<const uint8_t *>(steps_arr.data()),
                  steps_arr.size() * sizeof(int32_t)),
              "unable to set data for STEPS");

  FAIL_IF_ERR(guidance_scale_in_ptr->AppendRaw(
                  reinterpret_cast<const uint8_t *>(guidance_scales.data()),
                  guidance_scales.size() * sizeof(float)),
              "unable to set data for GUIDANCE_SCALES");

  FAIL_IF_ERR(
      seed_in_ptr->AppendRaw(reinterpret_cast<const uint8_t *>(seeds.data()),
                             seeds.size() * sizeof(int64_t)),
      "unable to set data for SEEDS");
  FAIL_IF_ERR(control_images_in_ptr->AppendRaw(control_image_input_data),
              "unable to set data for CONTROLNET_IMAGES");

  FAIL_IF_ERR(controlnet_scales_in_ptr->AppendRaw(
                  reinterpret_cast<const uint8_t *>(controlnet_scales.data()),
                  controlnet_scales.size() * sizeof(float)),
              "unable to set data for CONTROLNET_SCALES");

  // Create Requested Outputs
  inference_tc::InferRequestedOutput *images;
  FAIL_IF_ERR(inference_tc::InferRequestedOutput::Create(&images, "IMAGES"),
              "unable to set output data");
  std::shared_ptr<inference_tc::InferRequestedOutput> images_ptr;
  images_ptr.reset(images);

  // Set inputs and outputs
  auto inputs = {unet_model_in_ptr.get(),      prompt_in_ptr.get(),
                 negative_prompt_in_ptr.get(), samples_in_ptr.get(),
                 scheduler_in_ptr.get(),       steps_in_ptr.get(),
                 guidance_scale_in_ptr.get(),  seed_in_ptr.get(),
                 control_images_in_ptr.get(),  controlnet_scales_in_ptr.get()};

  std::vector<const inference_tc::InferRequestedOutput *> outputs = {
      images_ptr.get()};

  inference_tc::InferOptions options("stable_diffusion_controlnet");
  options.model_version_ = "1";

  // Set results and query the model
  inference_tc::InferResult *results;
  FAIL_IF_ERR(triton_client->Infer(&results, options, inputs, outputs),
              "unable to run model");
  std::shared_ptr<inference_tc::InferResult> results_ptr;
  results_ptr.reset(results);

  // Get the data type
  // std::string datatype;
  // results_ptr->Datatype("IMAGES", &datatype);

  const uint8_t *output_buffer;
  size_t output_byte_size;
  FAIL_IF_ERR(results_ptr->RawData("IMAGES", &output_buffer, &output_byte_size),
              "unable to get raw data for 'IMAGES'");

  size_t output_element_count = output_byte_size / sizeof(uint8_t);
  std::vector<uint8_t> output_data(output_element_count);
  memcpy(output_data.data(), output_buffer, output_byte_size);
  return output_data;
}

} // namespace inference
