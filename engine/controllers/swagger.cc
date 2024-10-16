#include "swagger.h"

const std::string SwaggerController::swaggerUIHTML = R"(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>Swagger UI</title>
    <link rel="stylesheet" type="text/css" href="https://cdnjs.cloudflare.com/ajax/libs/swagger-ui/4.18.3/swagger-ui.css" />
    <style>
        html { box-sizing: border-box; overflow: -moz-scrollbars-vertical; overflow-y: scroll; }
        *, *:before, *:after { box-sizing: inherit; }
        body { margin: 0; background: #fafafa; }
    </style>
</head>
<body>
    <div id="swagger-ui"></div>
    <script src="https://cdnjs.cloudflare.com/ajax/libs/swagger-ui/4.18.3/swagger-ui-bundle.js"></script>
    <script src="https://cdnjs.cloudflare.com/ajax/libs/swagger-ui/4.18.3/swagger-ui-standalone-preset.js"></script>
    <script>
    window.onload = function() {
        const ui = SwaggerUIBundle({
            url: "/openapi.json",
            dom_id: '#swagger-ui',
            deepLinking: true,
            presets: [
                SwaggerUIBundle.presets.apis,
                SwaggerUIStandalonePreset
            ],
            plugins: [
                SwaggerUIBundle.plugins.DownloadUrl
            ],
            layout: "StandaloneLayout"
        });
        window.ui = ui;
    };
    </script>
</body>
</html>
)";

Json::Value SwaggerController::generateOpenAPISpec() {
  Json::Value spec;
  spec["openapi"] = "3.0.0";
  spec["info"]["title"] = "Cortex API swagger";
  spec["info"]["version"] = "1.0.0";

  // Health Endpoint
  {
    Json::Value& path = spec["paths"]["/healthz"]["get"];
    path["summary"] = "Check system health";
    path["description"] = "Returns the health status of the cortex-cpp service";

    Json::Value& responses = path["responses"];
    responses["200"]["description"] = "Service is healthy";
    responses["200"]["content"]["text/html"]["schema"]["type"] = "string";
    responses["200"]["content"]["text/html"]["schema"]["example"] =
        "cortex-cpp is alive!!!";
  }

  // Engines endpoints
  // Install Engine
  {
    Json::Value& path = spec["paths"]["/v1/engines/install/{engine}"]["post"];
    path["summary"] = "Install an engine";
    path["parameters"][0]["name"] = "engine";
    path["parameters"][0]["in"] = "path";
    path["parameters"][0]["required"] = true;
    path["parameters"][0]["schema"]["type"] = "string";

    Json::Value& responses = path["responses"];
    responses["200"]["description"] = "Engine installed successfully";
    responses["200"]["content"]["application/json"]["schema"]["type"] =
        "object";
    responses["200"]["content"]["application/json"]["schema"]["properties"]
             ["message"]["type"] = "string";

    responses["400"]["description"] = "Bad request";
    responses["400"]["content"]["application/json"]["schema"]["type"] =
        "object";
    responses["400"]["content"]["application/json"]["schema"]["properties"]
             ["message"]["type"] = "string";

    responses["409"]["description"] = "Conflict";
    responses["409"]["content"]["application/json"]["schema"]["type"] =
        "object";
    responses["409"]["content"]["application/json"]["schema"]["properties"]
             ["message"]["type"] = "string";
  }

  // Uninstall Engine
  {
    Json::Value& path = spec["paths"]["/v1/engines/{engine}"]["delete"];
    path["summary"] = "Uninstall an engine";
    path["parameters"][0]["name"] = "engine";
    path["parameters"][0]["in"] = "path";
    path["parameters"][0]["required"] = true;
    path["parameters"][0]["schema"]["type"] = "string";

    Json::Value& responses = path["responses"];
    responses["200"]["description"] = "Engine uninstalled successfully";
    responses["200"]["content"]["application/json"]["schema"]["type"] =
        "object";
    responses["200"]["content"]["application/json"]["schema"]["properties"]
             ["message"]["type"] = "string";

    responses["400"]["description"] = "Bad request";
    responses["400"]["content"]["application/json"]["schema"]["type"] =
        "object";
    responses["400"]["content"]["application/json"]["schema"]["properties"]
             ["message"]["type"] = "string";
  }

  // List Engines
  {
    Json::Value& path = spec["paths"]["/v1/engines"]["get"];
    path["summary"] = "List all engines";

    Json::Value& response = path["responses"]["200"];
    response["description"] = "List of engines retrieved successfully";
    response["content"]["application/json"]["schema"]["type"] = "object";
    response["content"]["application/json"]["schema"]["properties"]["object"]
            ["type"] = "string";
    response["content"]["application/json"]["schema"]["properties"]["data"]
            ["type"] = "array";
    response["content"]["application/json"]["schema"]["properties"]["data"]
            ["items"]["type"] = "object";
    Json::Value& itemProperties =
        response["content"]["application/json"]["schema"]["properties"]["data"]
                ["items"]["properties"];
    itemProperties["name"]["type"] = "string";
    itemProperties["description"]["type"] = "string";
    itemProperties["version"]["type"] = "string";
    itemProperties["variant"]["type"] = "string";
    itemProperties["productName"]["type"] = "string";
    itemProperties["status"]["type"] = "string";
    response["content"]["application/json"]["schema"]["properties"]["result"]
            ["type"] = "string";
  }

  // Get Engine
  {
    Json::Value& path = spec["paths"]["/v1/engines/{engine}"]["get"];
    path["summary"] = "Get engine details";
    path["parameters"][0]["name"] = "engine";
    path["parameters"][0]["in"] = "path";
    path["parameters"][0]["required"] = true;
    path["parameters"][0]["schema"]["type"] = "string";

    Json::Value& responses = path["responses"];
    responses["200"]["description"] = "Engine details retrieved successfully";
    Json::Value& schema =
        responses["200"]["content"]["application/json"]["schema"];
    schema["type"] = "object";
    schema["properties"]["name"]["type"] = "string";
    schema["properties"]["description"]["type"] = "string";
    schema["properties"]["version"]["type"] = "string";
    schema["properties"]["variant"]["type"] = "string";
    schema["properties"]["productName"]["type"] = "string";
    schema["properties"]["status"]["type"] = "string";

    responses["400"]["description"] = "Engine not found";
    responses["400"]["content"]["application/json"]["schema"]["type"] =
        "object";
    responses["400"]["content"]["application/json"]["schema"]["properties"]
             ["message"]["type"] = "string";
  }

  // Models Endpoints
  {
    // PullModel
    Json::Value& pull = spec["paths"]["/v1/models/pull"]["post"];
    pull["summary"] = "Pull a model";
    pull["requestBody"]["content"]["application/json"]["schema"]["type"] =
        "object";
    pull["requestBody"]["content"]["application/json"]["schema"]["properties"]
        ["model"]["type"] = "string";
    pull["requestBody"]["content"]["application/json"]["schema"]["required"] =
        Json::Value(Json::arrayValue);
    pull["requestBody"]["content"]["application/json"]["schema"]["required"]
        .append("model");
    pull["responses"]["200"]["description"] = "Model start downloading";
    pull["responses"]["400"]["description"] = "Bad request";

    // ListModel
    Json::Value& list = spec["paths"]["/v1/models"]["get"];
    list["summary"] = "List all models";
    list["responses"]["200"]["description"] =
        "List of models retrieved successfully";
    list["responses"]["400"]["description"] =
        "Failed to get list model information";

    // GetModel
    Json::Value& get = spec["paths"]["/v1/models/{model}"]["get"];
    get["summary"] = "Get model details";
    get["parameters"][0]["name"] = "model";
    get["parameters"][0]["in"] = "path";
    get["parameters"][0]["required"] = true;
    get["parameters"][0]["schema"]["type"] = "string";

    Json::Value& responses = get["responses"];
    responses["200"]["description"] = "Model details retrieved successfully";
    Json::Value& schema =
        responses["200"]["content"]["application/json"]["schema"];
    responses["responses"]["400"]["description"] =
        "Failed to get model information";

    responses["400"]["description"] = "Failed to get model information";
    responses["400"]["content"]["application/json"]["schema"]["type"] =
        "object";
    responses["400"]["content"]["application/json"]["schema"]["properties"]
             ["message"]["type"] = "string";

    // UpdateModel Endpoint
    Json::Value& update = spec["paths"]["/v1/models/{model}"]["patch"];
    update["summary"] = "Update model details";
    update["description"] =
        "Update various attributes of a model based on the ModelConfig "
        "structure";
    update["parameters"][0]["name"] = "model";
    update["parameters"][0]["in"] = "path";
    update["parameters"][0]["required"] = true;
    update["parameters"][0]["schema"]["type"] = "string";

    Json::Value& updateSchema =
        update["requestBody"]["content"]["application/json"]["schema"];
    updateSchema["type"] = "object";

    Json::Value& properties = updateSchema["properties"];
    properties["model"]["type"] = "string";
    properties["model"]["description"] =
        "Unique identifier for the model (cannot be updated)";

    properties["name"]["type"] = "string";
    properties["name"]["description"] = "Name of the model";

    properties["version"]["type"] = "string";
    properties["version"]["description"] = "Version of the model";

    properties["stop"]["type"] = "array";
    properties["stop"]["items"]["type"] = "string";
    properties["stop"]["description"] = "List of stop sequences";

    properties["top_p"]["type"] = "number";
    properties["top_p"]["format"] = "float";
    properties["top_p"]["description"] = "Top-p sampling parameter";

    properties["temperature"]["type"] = "number";
    properties["temperature"]["format"] = "float";
    properties["temperature"]["description"] = "Temperature for sampling";

    properties["frequency_penalty"]["type"] = "number";
    properties["frequency_penalty"]["format"] = "float";
    properties["frequency_penalty"]["description"] =
        "Frequency penalty for sampling";

    properties["presence_penalty"]["type"] = "number";
    properties["presence_penalty"]["format"] = "float";
    properties["presence_penalty"]["description"] =
        "Presence penalty for sampling";

    properties["max_tokens"]["type"] = "integer";
    properties["max_tokens"]["description"] =
        "Maximum number of tokens to generate";

    properties["stream"]["type"] = "boolean";
    properties["stream"]["description"] = "Whether to stream the output";

    properties["ngl"]["type"] = "integer";
    properties["ngl"]["description"] = "Number of GPU layers";

    properties["ctx_len"]["type"] = "integer";
    properties["ctx_len"]["description"] = "Context length";

    properties["engine"]["type"] = "string";
    properties["engine"]["description"] = "Engine used for the model";

    properties["prompt_template"]["type"] = "string";
    properties["prompt_template"]["description"] = "Template for prompts";

    properties["system_template"]["type"] = "string";
    properties["system_template"]["description"] =
        "Template for system messages";

    properties["user_template"]["type"] = "string";
    properties["user_template"]["description"] = "Template for user messages";

    properties["ai_template"]["type"] = "string";
    properties["ai_template"]["description"] = "Template for AI responses";

    properties["os"]["type"] = "string";
    properties["os"]["description"] = "Operating system";

    properties["gpu_arch"]["type"] = "string";
    properties["gpu_arch"]["description"] = "GPU architecture";

    properties["quantization_method"]["type"] = "string";
    properties["quantization_method"]["description"] =
        "Method used for quantization";

    properties["precision"]["type"] = "string";
    properties["precision"]["description"] = "Precision of the model";

    properties["files"]["type"] = "array";
    properties["files"]["items"]["type"] = "string";
    properties["files"]["description"] =
        "List of files associated with the model";

    properties["seed"]["type"] = "integer";
    properties["seed"]["description"] = "Seed for random number generation";

    properties["dynatemp_range"]["type"] = "number";
    properties["dynatemp_range"]["format"] = "float";
    properties["dynatemp_range"]["description"] = "Dynamic temperature range";

    properties["dynatemp_exponent"]["type"] = "number";
    properties["dynatemp_exponent"]["format"] = "float";
    properties["dynatemp_exponent"]["description"] =
        "Dynamic temperature exponent";

    properties["top_k"]["type"] = "integer";
    properties["top_k"]["description"] = "Top-k sampling parameter";

    properties["min_p"]["type"] = "number";
    properties["min_p"]["format"] = "float";
    properties["min_p"]["description"] = "Minimum probability for sampling";

    properties["tfs_z"]["type"] = "number";
    properties["tfs_z"]["format"] = "float";
    properties["tfs_z"]["description"] = "TFS-Z parameter";

    properties["typ_p"]["type"] = "number";
    properties["typ_p"]["format"] = "float";
    properties["typ_p"]["description"] = "Typical p parameter";

    properties["repeat_last_n"]["type"] = "integer";
    properties["repeat_last_n"]["description"] =
        "Number of tokens to consider for repeat penalty";

    properties["repeat_penalty"]["type"] = "number";
    properties["repeat_penalty"]["format"] = "float";
    properties["repeat_penalty"]["description"] = "Penalty for repeated tokens";

    properties["mirostat"]["type"] = "boolean";
    properties["mirostat"]["description"] = "Whether to use Mirostat sampling";

    properties["mirostat_tau"]["type"] = "number";
    properties["mirostat_tau"]["format"] = "float";
    properties["mirostat_tau"]["description"] = "Mirostat tau parameter";

    properties["mirostat_eta"]["type"] = "number";
    properties["mirostat_eta"]["format"] = "float";
    properties["mirostat_eta"]["description"] = "Mirostat eta parameter";

    properties["penalize_nl"]["type"] = "boolean";
    properties["penalize_nl"]["description"] = "Whether to penalize newlines";

    properties["ignore_eos"]["type"] = "boolean";
    properties["ignore_eos"]["description"] =
        "Whether to ignore end-of-sequence token";

    properties["n_probs"]["type"] = "integer";
    properties["n_probs"]["description"] = "Number of probabilities to return";

    properties["min_keep"]["type"] = "integer";
    properties["min_keep"]["description"] = "Minimum number of tokens to keep";

    update["responses"]["200"]["description"] = "Model updated successfully";
    update["responses"]["200"]["content"]["application/json"]["schema"]
          ["$ref"] = "#/components/schemas/SuccessResponse";

    update["responses"]["400"]["description"] = "Failed to update model";
    update["responses"]["400"]["content"]["application/json"]["schema"]
          ["$ref"] = "#/components/schemas/ErrorResponse";

    // Define the schemas
    Json::Value& schemas = spec["components"]["schemas"];

    schemas["SuccessResponse"]["type"] = "object";
    schemas["SuccessResponse"]["properties"]["result"]["type"] = "string";
    schemas["SuccessResponse"]["properties"]["result"]["description"] =
        "Result of the operation";
    schemas["SuccessResponse"]["properties"]["modelHandle"]["type"] = "string";
    schemas["SuccessResponse"]["properties"]["modelHandle"]["description"] =
        "Handle of the affected model";
    schemas["SuccessResponse"]["properties"]["message"]["type"] = "string";
    schemas["SuccessResponse"]["properties"]["message"]["description"] =
        "Detailed message about the operation";

    schemas["ErrorResponse"]["type"] = "object";
    schemas["ErrorResponse"]["properties"]["result"]["type"] = "string";
    schemas["ErrorResponse"]["properties"]["result"]["description"] =
        "Error result";
    schemas["ErrorResponse"]["properties"]["modelHandle"]["type"] = "string";
    schemas["ErrorResponse"]["properties"]["modelHandle"]["description"] =
        "Handle of the model that caused the error";
    schemas["ErrorResponse"]["properties"]["message"]["type"] = "string";
    schemas["ErrorResponse"]["properties"]["message"]["description"] =
        "Detailed error message";

    // ImportModel
    Json::Value& import = spec["paths"]["/v1/models/import"]["post"];
    import["summary"] = "Import a model";
    import["requestBody"]["content"]["application/json"]["schema"]["type"] =
        "object";
    import["requestBody"]["content"]["application/json"]["schema"]["properties"]
          ["model"]["type"] = "string";
    import["requestBody"]["content"]["application/json"]["schema"]["properties"]
          ["modelPath"]["type"] = "string";
    import["requestBody"]["content"]["application/json"]["schema"]["required"] =
        Json::Value(Json::arrayValue);
    import["requestBody"]["content"]["application/json"]["schema"]["required"]
        .append("model");
    import["requestBody"]["content"]["application/json"]["schema"]["required"]
        .append("modelPath");
    import["responses"]["200"]["description"] = "Model imported successfully";
    import["responses"]["400"]["description"] = "Failed to import model";

    // DeleteModel
    Json::Value& del = spec["paths"]["/v1/models/{model}"]["delete"];
    del["summary"] = "Delete a model";
    del["parameters"][0]["name"] = "model";
    del["parameters"][0]["in"] = "path";
    del["parameters"][0]["required"] = true;
    del["parameters"][0]["schema"]["type"] = "string";
    del["responses"]["200"]["description"] = "Model deleted successfully";
    del["responses"]["400"]["description"] = "Failed to delete model";

    // SetModelAlias
    Json::Value& alias = spec["paths"]["/v1/models/alias"]["post"];
    alias["summary"] = "Set model alias";
    alias["requestBody"]["content"]["application/json"]["schema"]["type"] =
        "object";
    alias["requestBody"]["content"]["application/json"]["schema"]["properties"]
         ["model"]["type"] = "string";
    alias["requestBody"]["content"]["application/json"]["schema"]["properties"]
         ["modelAlias"]["type"] = "string";
    alias["requestBody"]["content"]["application/json"]["schema"]["required"] =
        Json::Value(Json::arrayValue);
    alias["requestBody"]["content"]["application/json"]["schema"]["required"]
        .append("model");
    alias["requestBody"]["content"]["application/json"]["schema"]["required"]
        .append("modelAlias");
    alias["responses"]["200"]["description"] = "Model alias set successfully";
    alias["responses"]["400"]["description"] = "Failed to set model alias";

    // Start Model
    Json::Value& start = spec["paths"]["/v1/models/start"]["post"];
    start["summary"] = "Start model";
    start["requestBody"]["content"]["application/json"]["schema"]["type"] =
        "object";
    start["requestBody"]["content"]["application/json"]["schema"]["properties"]
         ["model"]["type"] = "string";
    start["requestBody"]["content"]["application/json"]["schema"]["properties"]
         ["prompt_template"]["type"] = "string";
    start["requestBody"]["content"]["application/json"]["schema"]["required"] =
        Json::Value(Json::arrayValue);
    start["requestBody"]["content"]["application/json"]["schema"]["required"]
        .append("model");
    start["responses"]["200"]["description"] = "Start model successfully";
    start["responses"]["400"]["description"] = "Failed to start model";

    // Stop Model
    Json::Value& stop = spec["paths"]["/v1/models/stop"]["post"];
    stop["summary"] = "Stop model";
    stop["requestBody"]["content"]["application/json"]["schema"]["type"] =
        "object";
    stop["requestBody"]["content"]["application/json"]["schema"]["properties"]
        ["model"]["type"] = "string";
    stop["requestBody"]["content"]["application/json"]["schema"]["required"] =
        Json::Value(Json::arrayValue);
    stop["requestBody"]["content"]["application/json"]["schema"]["required"]
        .append("model");
    stop["responses"]["200"]["description"] = "Stop model successfully";
    stop["responses"]["400"]["description"] = "Failed to stop model";
  }

  // OpenAI Compatible Endpoints
  {
    // Chat Completions
    Json::Value& chat = spec["paths"]["/v1/chat/completions"]["post"];
    chat["summary"] = "Create chat completion";
    chat["description"] = "Creates a completion for the chat message";
    chat["requestBody"]["content"]["application/json"]["schema"]["$ref"] =
        "#/components/schemas/ChatCompletionRequest";
    chat["responses"]["200"]["description"] = "Successful response";
    chat["responses"]["200"]["content"]["application/json"]["schema"]["$ref"] =
        "#/components/schemas/ChatCompletionResponse";

    // List Models
    Json::Value& models = spec["paths"]["/v1/models"]["get"];
    models["summary"] = "List models";
    models["description"] = "Lists the currently available models";
    models["responses"]["200"]["description"] = "Successful response";
    models["responses"]["200"]["content"]["application/json"]["schema"]
          ["$ref"] = "#/components/schemas/ModelList";

    // Create Fine-tuning Job
    Json::Value& finetune = spec["paths"]["/v1/fine_tuning/job"]["post"];
    finetune["summary"] = "Create fine-tuning job";
    finetune["description"] = "Creates a job that fine-tunes a specified model";
    finetune["requestBody"]["content"]["application/json"]["schema"]["$ref"] =
        "#/components/schemas/FineTuningRequest";
    finetune["responses"]["200"]["description"] = "Successful response";
    finetune["responses"]["200"]["content"]["application/json"]["schema"]
            ["$ref"] = "#/components/schemas/FineTuningResponse";

    // Create Embeddings
    Json::Value& embed = spec["paths"]["/v1/embeddings"]["post"];
    embed["summary"] = "Create embeddings";
    embed["description"] =
        "Creates an embedding vector representing the input text";
    embed["requestBody"]["content"]["application/json"]["schema"]["$ref"] =
        "#/components/schemas/EmbeddingRequest";
    embed["responses"]["200"]["description"] = "Successful response";
    embed["responses"]["200"]["content"]["application/json"]["schema"]["$ref"] =
        "#/components/schemas/EmbeddingResponse";
  }

  // Custom Cortex Endpoints
  {
    // Chat Completion
    Json::Value& chat =
        spec["paths"]["/inferences/server/chat_completion"]["post"];
    chat["summary"] = "Create chat completion (Cortex)";
    chat["description"] =
        "Creates a completion for the chat message using Cortex engine";
    chat["requestBody"]["content"]["application/json"]["schema"]["$ref"] =
        "#/components/schemas/CortexChatCompletionRequest";
    chat["responses"]["200"]["description"] = "Successful response";
    chat["responses"]["200"]["content"]["application/json"]["schema"]["$ref"] =
        "#/components/schemas/CortexChatCompletionResponse";

    // Embedding
    Json::Value& embed = spec["paths"]["/inferences/server/embedding"]["post"];
    embed["summary"] = "Create embeddings (Cortex)";
    embed["description"] = "Creates an embedding vector using Cortex engine";
    embed["requestBody"]["content"]["application/json"]["schema"]["$ref"] =
        "#/components/schemas/CortexEmbeddingRequest";
    embed["responses"]["200"]["description"] = "Successful response";
    embed["responses"]["200"]["content"]["application/json"]["schema"]["$ref"] =
        "#/components/schemas/CortexEmbeddingResponse";

    // Load Model
    Json::Value& load = spec["paths"]["/inferences/server/loadmodel"]["post"];
    load["summary"] = "Load a model";
    load["description"] = "Loads a specified model into the engine";
    load["requestBody"]["content"]["application/json"]["schema"]["$ref"] =
        "#/components/schemas/LoadModelRequest";
    load["responses"]["200"]["description"] = "Model loaded successfully";
    load["responses"]["200"]["content"]["application/json"]["schema"]["$ref"] =
        "#/components/schemas/SuccessResponse";

    // Unload Model
    Json::Value& unload =
        spec["paths"]["/inferences/server/unloadmodel"]["post"];
    unload["summary"] = "Unload a model";
    unload["description"] = "Unloads a specified model from the engine";
    unload["requestBody"]["content"]["application/json"]["schema"]["$ref"] =
        "#/components/schemas/UnloadModelRequest";
    unload["responses"]["200"]["description"] = "Model unloaded successfully";
    unload["responses"]["200"]["content"]["application/json"]["schema"]
          ["$ref"] = "#/components/schemas/SuccessResponse";

    // Model Status
    Json::Value& status =
        spec["paths"]["/inferences/server/modelstatus"]["post"];
    status["summary"] = "Get model status";
    status["description"] = "Retrieves the status of a specified model";
    status["requestBody"]["content"]["application/json"]["schema"]["$ref"] =
        "#/components/schemas/ModelStatusRequest";
    status["responses"]["200"]["description"] =
        "Model status retrieved successfully";
    status["responses"]["200"]["content"]["application/json"]["schema"]
          ["$ref"] = "#/components/schemas/ModelStatusResponse";

    // Get Models
    Json::Value& getModels = spec["paths"]["/inferences/server/models"]["get"];
    getModels["summary"] = "Get all models (Cortex)";
    getModels["description"] =
        "Retrieves a list of all available models in Cortex";
    getModels["responses"]["200"]["description"] =
        "Models retrieved successfully";
    getModels["responses"]["200"]["content"]["application/json"]["schema"]
             ["$ref"] = "#/components/schemas/CortexModelList";

    // Get Engines
    Json::Value& getEngines =
        spec["paths"]["/inferences/server/engines"]["get"];
    getEngines["summary"] = "Get all engines";
    getEngines["description"] = "Retrieves a list of all available engines";
    getEngines["responses"]["200"]["description"] =
        "Engines retrieved successfully";
    getEngines["responses"]["200"]["content"]["application/json"]["schema"]
              ["$ref"] = "#/components/schemas/EngineList";

    // Fine Tuning
    Json::Value& fineTuning =
        spec["paths"]["/inferences/server/finetuning"]["post"];
    fineTuning["summary"] = "Create fine-tuning job (Cortex)";
    fineTuning["description"] =
        "Creates a job that fine-tunes a specified model using Cortex engine";
    fineTuning["requestBody"]["content"]["application/json"]["schema"]["$ref"] =
        "#/components/schemas/CortexFineTuningRequest";
    fineTuning["responses"]["200"]["description"] =
        "Fine-tuning job created successfully";
    fineTuning["responses"]["200"]["content"]["application/json"]["schema"]
              ["$ref"] = "#/components/schemas/CortexFineTuningResponse";

    // Unload Engine
    Json::Value& unloadEngine =
        spec["paths"]["/inferences/server/unloadengine"]["post"];
    unloadEngine["summary"] = "Unload an engine";
    unloadEngine["description"] = "Unloads a specified engine";
    unloadEngine["requestBody"]["content"]["application/json"]["schema"]
                ["$ref"] = "#/components/schemas/UnloadEngineRequest";
    unloadEngine["responses"]["200"]["description"] =
        "Engine unloaded successfully";
    unloadEngine["responses"]["200"]["content"]["application/json"]["schema"]
                ["$ref"] = "#/components/schemas/SuccessResponse";
  }

  // Define schemas
  Json::Value& schemas = spec["components"]["schemas"];

  schemas["ChatCompletionRequest"]["type"] = "object";
  schemas["ChatCompletionRequest"]["properties"]["model"]["type"] = "string";
  schemas["ChatCompletionRequest"]["properties"]["messages"]["type"] = "array";
  schemas["ChatCompletionRequest"]["properties"]["messages"]["items"]["$ref"] =
      "#/components/schemas/ChatMessage";
  schemas["ChatCompletionRequest"]["properties"]["stream"]["type"] = "boolean";
  schemas["ChatCompletionRequest"]["properties"]["engine"]["type"] = "string";
  schemas["ChatCompletionRequest"]["properties"]["tools"]["type"] = "array";
  schemas["ChatCompletionRequest"]["properties"]["tools"]["items"]["$ref"] =
      "#/components/schemas/ToolsCall";
  schemas["ChatCompletionRequest"]["properties"]["tools_call_in_user_message"]
         ["type"] = "boolean";
  schemas["ChatCompletionRequest"]["properties"]["tools_call_in_user_message"]
         ["default"] = false;
  schemas["ToolsCall"]["type"] = "object";

  schemas["ChatMessage"]["type"] = "object";
  schemas["ChatMessage"]["properties"]["role"]["type"] = "string";
  schemas["ChatMessage"]["properties"]["content"]["type"] = "string";
  schemas["ChatMessage"]["properties"]["tools"]["type"] = "array";
  schemas["ChatMessage"]["properties"]["tools"]["items"]["$ref"] =
      "#/components/schemas/ToolsCall";

  schemas["ChatCompletionResponse"]["type"] = "object";
  // Add properties based on your implementation

  schemas["ModelList"]["type"] = "object";
  schemas["ModelList"]["properties"]["object"]["type"] = "string";
  schemas["ModelList"]["properties"]["data"]["type"] = "array";
  schemas["ModelList"]["properties"]["data"]["items"]["$ref"] =
      "#/components/schemas/Model";

  schemas["Model"]["type"] = "object";
  schemas["Model"]["properties"]["id"]["type"] = "string";
  schemas["Model"]["properties"]["object"]["type"] = "string";

  schemas["FineTuningRequest"]["type"] = "object";
  schemas["FineTuningRequest"]["properties"]["model"]["type"] = "string";
  schemas["FineTuningRequest"]["properties"]["training_file"]["type"] =
      "string";

  schemas["FineTuningResponse"]["type"] = "object";
  // Add properties based on your implementation

  schemas["EmbeddingRequest"]["type"] = "object";
  schemas["EmbeddingRequest"]["properties"]["model"]["type"] = "string";
  schemas["EmbeddingRequest"]["properties"]["input"]["type"] = "string";

  schemas["EmbeddingResponse"]["type"] = "object";
  schemas["EmbeddingResponse"]["properties"]["object"]["type"] = "string";
  schemas["EmbeddingResponse"]["properties"]["data"]["type"] = "array";
  schemas["EmbeddingResponse"]["properties"]["data"]["items"]["type"] =
      "object";
  schemas["EmbeddingResponse"]["properties"]["data"]["items"]["properties"]
         ["embedding"]["type"] = "array";
  schemas["EmbeddingResponse"]["properties"]["data"]["items"]["properties"]
         ["embedding"]["items"]["type"] = "number";

  schemas["CortexChatCompletionRequest"]["type"] = "object";
  schemas["CortexChatCompletionRequest"]["properties"]["engine"]["type"] =
      "string";
  schemas["CortexChatCompletionRequest"]["properties"]["model"]["type"] =
      "string";
  schemas["CortexChatCompletionRequest"]["properties"]["messages"]["type"] =
      "array";
  schemas["CortexChatCompletionRequest"]["properties"]["messages"]["items"]
         ["$ref"] = "#/components/schemas/ChatMessage";
  schemas["CortexChatCompletionRequest"]["properties"]["stream"]["type"] =
      "boolean";
  // Add other properties based on your implementation

  schemas["CortexChatCompletionResponse"]["type"] = "object";
  // Add properties based on your implementation

  schemas["CortexEmbeddingRequest"]["type"] = "object";
  schemas["CortexEmbeddingRequest"]["properties"]["engine"]["type"] = "string";
  schemas["CortexEmbeddingRequest"]["properties"]["input"]["type"] = "string";

  schemas["CortexEmbeddingResponse"]["type"] = "object";
  // Add properties based on your implementation

  schemas["LoadModelRequest"]["type"] = "object";
  schemas["LoadModelRequest"]["properties"]["engine"]["type"] = "string";
  schemas["LoadModelRequest"]["properties"]["model_path"]["type"] = "string";
  schemas["LoadModelRequest"]["properties"]["model"]["type"] = "string";
  schemas["LoadModelRequest"]["properties"]["engine"]["type"] = "string";
  schemas["LoadModelRequest"]["properties"]["stop"]["type"] = "array";
  schemas["LoadModelRequest"]["properties"]["stop"]["items"]["type"] = "string";
  schemas["LoadModelRequest"]["properties"]["stop"]["description"] =
      "List of stop sequences";

  schemas["LoadModelRequest"]["properties"]["stream"]["type"] = "boolean";
  schemas["LoadModelRequest"]["properties"]["stream"]["description"] =
      "Whether to stream the output";

  schemas["LoadModelRequest"]["properties"]["ngl"]["type"] = "integer";
  schemas["LoadModelRequest"]["properties"]["ngl"]["description"] =
      "Number of GPU layers";

  schemas["LoadModelRequest"]["properties"]["ctx_len"]["type"] = "integer";
  schemas["LoadModelRequest"]["properties"]["ctx_len"]["description"] =
      "Context length";

  schemas["LoadModelRequest"]["properties"]["engine"]["type"] = "string";
  schemas["LoadModelRequest"]["properties"]["engine"]["description"] =
      "Engine used for the model";

  schemas["LoadModelRequest"]["properties"]["system_template"]["type"] =
      "string";
  schemas["LoadModelRequest"]["properties"]["system_template"]["description"] =
      "Template for system messages";

  schemas["LoadModelRequest"]["properties"]["user_template"]["type"] = "string";
  schemas["LoadModelRequest"]["properties"]["user_template"]["description"] =
      "Template for user messages";

  schemas["LoadModelRequest"]["properties"]["ai_template"]["type"] = "string";
  schemas["LoadModelRequest"]["properties"]["ai_template"]["description"] =
      "Template for AI responses";

  schemas["LoadModelRequest"]["properties"]["n_probs"]["type"] = "integer";
  schemas["LoadModelRequest"]["properties"]["n_probs"]["description"] =
      "Number of probabilities to return";

  // Add other properties based on your implementation

  schemas["UnloadModelRequest"]["type"] = "object";
  schemas["UnloadModelRequest"]["properties"]["engine"]["type"] = "string";
  schemas["UnloadModelRequest"]["properties"]["model"]["type"] = "string";
  // Add other properties based on your implementation

  schemas["ModelStatusRequest"]["type"] = "object";
  schemas["ModelStatusRequest"]["properties"]["engine"]["type"] = "string";
  schemas["ModelStatusRequest"]["properties"]["model"]["type"] = "string";
  // Add other properties based on your implementation

  schemas["ModelStatusResponse"]["type"] = "object";
  // Add properties based on your implementation

  schemas["CortexModelList"]["type"] = "object";
  schemas["CortexModelList"]["properties"]["data"]["type"] = "array";
  schemas["CortexModelList"]["properties"]["data"]["items"]["$ref"] =
      "#/components/schemas/CortexModel";

  schemas["CortexModel"]["type"] = "object";
  // Add properties based on your implementation

  schemas["EngineList"]["type"] = "object";
  schemas["EngineList"]["properties"]["object"]["type"] = "string";
  schemas["EngineList"]["properties"]["data"]["type"] = "array";
  schemas["EngineList"]["properties"]["data"]["items"]["$ref"] =
      "#/components/schemas/Engine";

  schemas["Engine"]["type"] = "object";
  schemas["Engine"]["properties"]["id"]["type"] = "string";
  schemas["Engine"]["properties"]["object"]["type"] = "string";

  schemas["CortexFineTuningRequest"]["type"] = "object";
  schemas["CortexFineTuningRequest"]["properties"]["engine"]["type"] = "string";
  // Add other properties based on your implementation

  schemas["CortexFineTuningResponse"]["type"] = "object";
  // Add properties based on your implementation

  schemas["UnloadEngineRequest"]["type"] = "object";
  schemas["UnloadEngineRequest"]["properties"]["engine"]["type"] = "string";

  schemas["SuccessResponse"]["type"] = "object";
  schemas["SuccessResponse"]["properties"]["message"]["type"] = "string";
  // TODO: Add more paths and details based on your API

  return spec;
}

void SwaggerController::serveSwaggerUI(
    const drogon::HttpRequestPtr& req,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback) const {
  auto resp = drogon::HttpResponse::newHttpResponse();
  resp->setBody(swaggerUIHTML);
  resp->setContentTypeCode(drogon::CT_TEXT_HTML);
  callback(resp);
}

void SwaggerController::serveOpenAPISpec(
    const drogon::HttpRequestPtr& req,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback) const {
  Json::Value spec = generateOpenAPISpec();
  auto resp = drogon::HttpResponse::newHttpJsonResponse(spec);
  callback(resp);
}