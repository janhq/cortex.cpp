# Inference Backend Implementation

## Directory Structure

```
.
├── CMakeLists.txt
├── config.yaml
├── controllers -> controller for each path
│   ├── llm_models
│   │   ├── inferences_llm_models.cc
│   │   └── inferences_llm_models.h
│   └── txt2img
│       ├── inferences_txt2img.cc
│       └── inferences_txt2img.h
├── include
│   ├── backend_utils.h
│   ├── inference_lib.h -> Store the main inference logic to triton
│   └── validation_macro.h
├── main.cc
├── README.md
├── schemas
│   ├── openai_compatible_schema.json
│   └── txt2img.json
└── test
    ├── CMakeLists.txt
    └── test_main.cc
```

## Details of implementation

As can be seen from the above docs include folder will mostly store the header only library/include to support most of the logic inside the controller. Below is the documentation of some of the functions that are inside **inference_lib.h**

# inference_lib.h

The code provided includes several functions and classes related to inference and text completion. Below is the documentation for each function and class:

### Namespace `inference_utils`

#### Function `loadSchema`

```cpp
void loadSchema(const std::string &schemaFilePath, valijson::Schema &targetSchema)
```

**Input:**
- `schemaFilePath` (string): Path to the schema file.
- `targetSchema` (valijson::Schema): Target schema object.

**Output:**
- None

This function loads a JSON schema document from the specified file and populates the target schema object.

#### Function `validate_json`

```cpp
bool validate_json(const valijson::Schema &schema, const Json::Value &jsonBody,
                   valijson::Validator &validator)
```

**Input:**
- `schema` (valijson::Schema): Schema object for validation.
- `jsonBody` (Json::Value): JSON body to be validated.
- `validator` (valijson::Validator): Validator object for validation.

**Output:**
- `true` if JSON body is valid according to the schema, `false` otherwise.

This function validates a JSON body against a given schema using the specified validator object. It returns `true` if the JSON body is valid, and `false` otherwise.

#### Function `generate_random_positive_int`

```cpp
int generate_random_positive_int(int upper_limit)
```

**Input:**
- `upper_limit` (int): Upper limit for generating random positive integer.

**Output:**
- Random positive integer between 1 and `upper_limit`.

This function generates a random positive integer within the specified upper limit.

#### Function `generate_random_string`

```cpp
std::string generate_random_string(std::size_t length)
```

**Input:**
- `length` (std::size_t): Length of the random string to generate.

**Output:**
- Random string of the specified length.

This function generates a random string of the specified length using a combination of alphanumeric characters.

#### Function `pad_to_square`

```cpp
cv::Mat pad_to_square(const cv::Mat &image)
```

**Input:**
- `image` (cv::Mat): Input image to be padded.

**Output:**
- Padded image (cv::Mat) with square dimensions.

This function takes an input image and pads it to create a square image by adding white background. The output image has the dimensions of the largest side of the input image.

### Namespace `inference`

#### Class `TextCompletions`

##### Method `InitializeClient`

```cpp
void InitializeClient()
```

**Input:**
- None

**Output:**
- None

This method initializes the gRPC client for the TextCompletions class. It creates the triton_client object and loads the sentencepiece model.

##### Method `GetCompletions`

```cpp
void GetCompletions(std::string model_name, std::string text_prompt,
                    int max_tokens, float top_p, float temperature,
                    uint64_t sequence_id, bool sequence_start,
                    int stream_timeout)
```

**Input:**
- `model_name` (std::string): Name of the model to use for completions.
- `text_prompt` (std::string): Text prompt for generating completions.
- `max_tokens` (int): Maximum number of tokens to generate.
- `top_p` (float): Top-p sampling probability.
- `temperature` (float): Temperature value for softmax sampling.
- `sequence_id` (uint64_t): Sequence ID for tracking completions.
- `sequence_start` (bool): Whether it is the start of a new sequence.
- `stream_timeout` (int): Timeout value for the gRPC stream connection.

**Output:**
- None

This method generates text completions using the specified model and parameters. It sends the request to the Triton server using gRPC streaming and receives the completions in chunks asynchronously. The completions are stored in the `result_list` vector of the TextCompletions class.

#### Template Function `process_image`

```cpp
template <typename T>
std::vector<uchar> process_image(std::vector<T> &data, const std::vector<int> &shape)
```

**Input:**
- `data` (std::vector<T>): Input image data.
- `shape` (std::vector<int>): Shape of the image data.

**Output:**
- Encoded image data as a vector of `uchar` type.

This template function takes image data and its shape as input, and processes the image before encoding it into PNG format. The function supports both float and uint8_t image data types.

#### Function `infer_txt2img`

```cpp
std::vector<uint8_t> infer_txt2img(
    std::unique_ptr<inference_tc::InferenceServerGrpcClient> &triton_client,
    std::string prompt, std::string negative_prompt, int width, int height,
    int steps, int seed, bool compel, std::string model_name)
```

**Input:**
- `triton_client` (std::unique_ptr<inference_tc::InferenceServerGrpcClient>&): Triton gRPC client object.
- `prompt` (std::string): Text prompt for the text-to-image generation.
- `negative_prompt` (std::string): Negative text prompt for the text-to-image generation.
- `width` (int): Width of the output image.
- `height` (int): Height of the output image.
- `steps` (int): Number of steps for the image generation process.
- `seed` (int): Random seed value for reproducibility.
- `compel` (bool): Specify whether to use compel or not.
- `model_name` (std::string): Name of the model to use for text-to-image generation.

**Output:**
- Generated image data as a vector of `uint8_t` type.

This function sends a request to the Triton server to generate an image from text using the specified model and parameters. It returns the generated image data as a vector of `uint8_t`.