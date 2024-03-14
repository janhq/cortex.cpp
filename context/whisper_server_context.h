#pragma once
#include <mutex>
#include <vector>
#include <utility>
#include <string>
#include <thread>

#include "whisper.h"

// Terminal color map. 10 colors grouped in ranges [0.0, 0.1, ..., 0.9]
// Lowest is red, middle is yellow, highest is green.
const std::vector<std::string> k_colors = {
    "\033[38;5;196m", "\033[38;5;202m", "\033[38;5;208m", "\033[38;5;214m",
    "\033[38;5;220m", "\033[38;5;226m", "\033[38;5;190m", "\033[38;5;154m",
    "\033[38;5;118m", "\033[38;5;82m",
};

// output formats
const std::string json_format = "json";
const std::string text_format = "text";
const std::string srt_format = "srt";
const std::string vjson_format = "verbose_json";
const std::string vtt_format = "vtt";

#define COMMON_SAMPLE_RATE 16000

struct whisper_params {
  int32_t n_threads =
      (std::min)(4, (int32_t)std::thread::hardware_concurrency());
  int32_t n_processors = 1;
  int32_t offset_t_ms = 0;
  int32_t offset_n = 0;
  int32_t duration_ms = 0;
  int32_t progress_step = 5;
  int32_t max_context = -1;
  int32_t max_len = 0;
  int32_t best_of = 2;
  int32_t beam_size = -1;

  float word_thold = 0.01f;
  float entropy_thold = 2.40f;
  float logprob_thold = -1.00f;
  float temperature = 0.00f;
  float temperature_inc = 0.20f;

  bool speed_up = false;
  bool debug_mode = false;
  bool translate = false;
  bool detect_language = false;
  bool diarize = false;
  bool tinydiarize = false;
  bool split_on_word = false;
  bool no_fallback = false;
  bool print_special = false;
  bool print_colors = false;
  bool print_realtime = false;
  bool print_progress = false;
  bool no_timestamps = false;
  bool use_gpu = true;
  bool ffmpeg_converter = false;

  std::string language = "en";
  std::string prompt = "";
  std::string font_path =
      "/System/Library/Fonts/Supplemental/Courier New Bold.ttf";
  std::string model = "models/ggml-base.en.bin";

  std::string response_format = json_format;

  // [TDRZ] speaker turn string
  std::string tdrz_speaker_turn =
      " [SPEAKER_TURN]";  // TODO: set from command line

  std::string openvino_encode_device = "CPU";
};

// Read WAV audio file and store the PCM data into pcmf32
// The sample rate of the audio must be equal to COMMON_SAMPLE_RATE
// If stereo flag is set and the audio has 2 channels, the pcmf32s will contain
// 2 channel PCM
bool read_wav(const std::string& fname, std::vector<float>& pcmf32,
              std::vector<std::vector<float>>& pcmf32s, bool stereo);

std::string output_str(struct whisper_context* ctx,
                       const whisper_params& params,
                       std::vector<std::vector<float>> pcmf32s);

std::string estimate_diarization_speaker(
    std::vector<std::vector<float>> pcmf32s, int64_t t0, int64_t t1,
    bool id_only = false);

//  500 -> 00:05.000
// 6000 -> 01:00.000
std::string to_timestamp(int64_t t, bool comma = false);

int timestamp_to_sample(int64_t t, int n_samples);

bool is_file_exist(const char* fileName);

void whisper_print_usage(int /*argc*/, char** argv,
                         const whisper_params& params);

bool whisper_params_parse(int argc, char** argv, whisper_params& params);

void check_ffmpeg_availibility();

bool convert_to_wav(const std::string& temp_filename, std::string& error_resp);

void whisper_print_progress_callback(struct whisper_context* /*ctx*/,
                                     struct whisper_state* /*state*/,
                                     int progress, void* user_data);

void whisper_print_segment_callback(struct whisper_context* ctx,
                                    struct whisper_state* /*state*/, int n_new,
                                    void* user_data);

struct whisper_print_user_data {
  const whisper_params* params;

  const std::vector<std::vector<float>>* pcmf32s;
  int progress_prev;
};

struct whisper_server_context {
  whisper_params params;
  whisper_params default_params;
  std::mutex whisper_mutex;
  std::string model_id;

  struct whisper_context_params cparams;
  struct whisper_context* ctx = nullptr;

  whisper_server_context() = default;  // add this line

  // Constructor
  whisper_server_context(const std::string& model_id) {
    this->model_id = model_id;
    this->cparams = whisper_context_params();
    this->ctx = nullptr;
    // store default params so we can reset after each inference request
    this->default_params = whisper_params();
    this->params = whisper_params();
  }

  // Move constructor
  whisper_server_context(whisper_server_context&& other) noexcept
      : params(std::move(other.params)),
        default_params(std::move(other.default_params)),
        whisper_mutex()  // std::mutex is not movable, so we initialize a new one
        ,
        model_id(std::move(other.model_id)),
        cparams(std::move(other.cparams)),
        ctx(std::exchange(
            other.ctx,
            nullptr))  // ctx is a raw pointer, so we use std::exchange
  {}

  bool load_model(std::string& model_path);

  std::string inference(std::string& input_file_path, std::string languague,
                        std::string prompt, std::string response_format,
                        float temperature, bool translate);

  ~whisper_server_context();
};