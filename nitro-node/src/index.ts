import os from "node:os";
import fs from "node:fs";
import path from "node:path";
import { ChildProcessWithoutNullStreams, spawn } from "node:child_process";
import tcpPortUsed from "tcp-port-used";
import fetchRT from "fetch-retry";
import osUtils from "os-utils";
import { getNitroProcessInfo, updateNvidiaInfo } from "./nvidia";
import { executableNitroFile } from "./execute";
import downloadNitro from '../scripts/download-nitro';
// Polyfill fetch with retry
const fetchRetry = fetchRT(fetch);

// The PORT to use for the Nitro subprocess
const PORT = 3928;
// The HOST address to use for the Nitro subprocess
const LOCAL_HOST = "127.0.0.1";
// The URL for the Nitro subprocess
const NITRO_HTTP_SERVER_URL = `http://${LOCAL_HOST}:${PORT}`;
// The URL for the Nitro subprocess to load a model
const NITRO_HTTP_LOAD_MODEL_URL = `${NITRO_HTTP_SERVER_URL}/inferences/llamacpp/loadmodel`;
// The URL for the Nitro subprocess to validate a model
const NITRO_HTTP_VALIDATE_MODEL_URL = `${NITRO_HTTP_SERVER_URL}/inferences/llamacpp/modelstatus`;
// The URL for the Nitro subprocess to kill itself
const NITRO_HTTP_KILL_URL = `${NITRO_HTTP_SERVER_URL}/processmanager/destroy`;
// The URL for the Nitro subprocess to run chat completion
const NITRO_HTTP_CHAT_URL = `${NITRO_HTTP_SERVER_URL}/inferences/llamacpp/chat_completion`;

// The default config for using Nvidia GPU
const NVIDIA_DEFAULT_CONFIG: NitroNvidiaConfig = {
  notify: true,
  run_mode: "cpu",
  nvidia_driver: {
    exist: false,
    version: "",
  },
  cuda: {
    exist: false,
    version: "",
  },
  gpus: [],
  gpu_highest_vram: "",
};

// The supported model format
// TODO: Should be an array to support more models
const SUPPORTED_MODEL_FORMATS = [".gguf"];

// The subprocess instance for Nitro
let subprocess: ChildProcessWithoutNullStreams | undefined = undefined;
// The current model file url
let currentModelFile: string = "";
// The current model settings
let currentSettings: NitroModelSetting | undefined = undefined;
// The Nvidia info file for checking for CUDA support on the system
let nvidiaConfig: NitroNvidiaConfig = NVIDIA_DEFAULT_CONFIG;
// The logger to use, default to stdout
let log: NitroLogger = (message, ..._) =>
  process.stdout.write(message + os.EOL);

/**
 * Get current Nvidia config
 * @returns {NitroNvidiaConfig} A copy of the config object
 * The returned object should be used for reading only
 * Writing to config should be via the function {@setNvidiaConfig}
 */
function getNvidiaConfig(): NitroNvidiaConfig {
  return Object.assign({}, nvidiaConfig);
}

/**
 * Set custom Nvidia config for running inference over GPU
 * @param {NitroNvidiaConfig} config The new config to apply
 */
function setNvidiaConfig(config: NitroNvidiaConfig) {
  nvidiaConfig = config;
}

/**
 * Set logger before running nitro
 * @param {NitroLogger} logger The logger to use
 */
function setLogger(logger: NitroLogger) {
  log = logger;
}

/**
 * Stops a Nitro subprocess.
 * @param wrapper - The model wrapper.
 * @returns A Promise that resolves when the subprocess is terminated successfully, or rejects with an error message if the subprocess fails to terminate.
 */
function stopModel(): Promise<NitroModelOperationResponse> {
  return killSubprocess();
}

/**
 * Initializes a Nitro subprocess to load a machine learning model.
 * @param modelFullPath - The absolute full path to model directory.
 * @param promptTemplate - The template to use for generating prompts.
 * @returns A Promise that resolves when the model is loaded successfully, or rejects with an error message if the model is not found or fails to load.
 */
async function runModel({
  modelFullPath,
  promptTemplate,
}: NitroModelInitOptions): Promise<NitroModelOperationResponse | Error> {
  // Download nitro binaries if it's not already downloaded
  await downloadNitro();
  const files: string[] = fs.readdirSync(modelFullPath);

  // Look for model file with supported format
  const ggufBinFile = files.find(
    (file) =>
      file === path.basename(modelFullPath) ||
      SUPPORTED_MODEL_FORMATS.some((ext) => file.toLowerCase().endsWith(ext)),
  );

  if (!ggufBinFile) return Promise.reject("No GGUF model file found");

  currentModelFile = path.join(modelFullPath, ggufBinFile);

  const nitroResourceProbe = await getResourcesInfo();
  // Convert promptTemplate to system_prompt, user_prompt, ai_prompt
  const prompt: NitroPromptSetting = {};
  if (promptTemplate) {
    try {
      Object.assign(prompt, promptTemplateConverter(promptTemplate));
    } catch (e: any) {
      return Promise.reject(e);
    }
  }

  currentSettings = {
    ...prompt,
    llama_model_path: currentModelFile,
    // This is critical and requires real system information
    cpu_threads: Math.max(
      1,
      Math.round(nitroResourceProbe.numCpuPhysicalCore / 2),
    ),
  };
  return runNitroAndLoadModel();
}

/**
 * 1. Spawn Nitro process
 * 2. Load model into Nitro subprocess
 * 3. Validate model status
 * @returns
 */
async function runNitroAndLoadModel(): Promise<
  NitroModelOperationResponse | { error: any }
> {
  // Gather system information for CPU physical cores and memory
  return killSubprocess()
    .then(() => tcpPortUsed.waitUntilFree(PORT, 300, 5000))
    .then(() => {
      /**
       * There is a problem with Windows process manager
       * Should wait for awhile to make sure the port is free and subprocess is killed
       * The tested threshold is 500ms
       **/
      if (process.platform === "win32") {
        return new Promise((resolve) => setTimeout(() => resolve({}), 500));
      } else {
        return Promise.resolve({});
      }
    })
    .then(spawnNitroProcess)
    .then(() => loadLLMModel(currentSettings))
    .then(validateModelStatus)
    .catch((err) => {
      // TODO: Broadcast error so app could display proper error message
      log(`[NITRO]::Error: ${err}`);
      return { error: err };
    });
}

/**
 * Parse prompt template into agrs settings
 * @param {string} promptTemplate Template as string
 * @returns {(NitroPromptSetting | never)} parsed prompt setting
 * @throws {Error} if cannot split promptTemplate
 */
function promptTemplateConverter(
  promptTemplate: string,
): NitroPromptSetting | never {
  // Split the string using the markers
  const systemMarker = "{system_message}";
  const promptMarker = "{prompt}";

  if (
    promptTemplate.includes(systemMarker) &&
    promptTemplate.includes(promptMarker)
  ) {
    // Find the indices of the markers
    const systemIndex = promptTemplate.indexOf(systemMarker);
    const promptIndex = promptTemplate.indexOf(promptMarker);

    // Extract the parts of the string
    const system_prompt = promptTemplate.substring(0, systemIndex);
    const user_prompt = promptTemplate.substring(
      systemIndex + systemMarker.length,
      promptIndex,
    );
    const ai_prompt = promptTemplate.substring(
      promptIndex + promptMarker.length,
    );

    // Return the split parts
    return { system_prompt, user_prompt, ai_prompt };
  } else if (promptTemplate.includes(promptMarker)) {
    // Extract the parts of the string for the case where only promptMarker is present
    const promptIndex = promptTemplate.indexOf(promptMarker);
    const user_prompt = promptTemplate.substring(0, promptIndex);
    const ai_prompt = promptTemplate.substring(
      promptIndex + promptMarker.length,
    );

    // Return the split parts
    return { user_prompt, ai_prompt };
  }

  // Throw error if none of the conditions are met
  throw Error("Cannot split prompt template");
}

/**
 * Loads a LLM model into the Nitro subprocess by sending a HTTP POST request.
 * @returns A Promise that resolves when the model is loaded successfully, or rejects with an error message if the model is not found or fails to load.
 */
async function loadLLMModel(settings: any): Promise<Response> {
  log(`[NITRO]::Debug: Loading model with params ${JSON.stringify(settings)}`);
  try {
    const res = await fetchRetry(NITRO_HTTP_LOAD_MODEL_URL, {
      method: "POST",
      headers: {
        "Content-Type": "application/json",
      },
      body: JSON.stringify(settings),
      retries: 3,
      retryDelay: 500,
    });
    // FIXME: Actually check response, as the model directory might not exist
    log(
      `[NITRO]::Debug: Load model success with response ${JSON.stringify(res)}`,
    );
    return await Promise.resolve(res);
  } catch (err) {
    log(`[NITRO]::Error: Load model failed with error ${err}`);
    return await Promise.reject();
  }
}

/**
 * Run chat completion by sending a HTTP POST request and stream the response if outStream is specified
 * @param {any} request The request that is then sent to nitro
 * @param {WritableStream} outStream Optional stream that consume the response body
 * @returns {Promise<Response>} A Promise that resolves when the chat completion success, or rejects with an error if the completion fails.
 * @description If outStream is specified, the response body is consumed and cannot be used to reconstruct the data
 */
async function chatCompletion(
  request: any,
  outStream?: WritableStream,
): Promise<Response> {
  if (outStream) {
    // Add stream option if there is an outStream specified when calling this function
    Object.assign(request, {
      stream: true,
    });
  }
  log(
    `[NITRO]::Debug: Running chat completion with request ${JSON.stringify(request)}`,
  );
  return fetchRetry(NITRO_HTTP_CHAT_URL, {
    method: "POST",
    headers: {
      "Content-Type": "application/json",
      "Access-Control-Allow-Origin": "*",
      Accept: Boolean(outStream) ? "text/event-stream" : "application/json",
    },
    body: JSON.stringify(request),
    retries: 3,
    retryDelay: 500,
  })
    .then(async (response) => {
      if (outStream) {
        if (!response.body) {
          throw new Error("Error running chat completion");
        }
        const outPipe = response.body
          .pipeThrough(new TextDecoderStream())
          .pipeTo(outStream);
        // Wait for all the streams to complete before returning from async function
        await outPipe;
      }
      log(`[NITRO]::Debug: Chat completion success`);
      return response;
    })
    .catch((err) => {
      log(`[NITRO]::Error: Chat completion failed with error ${err}`);
      throw err;
    });
}

/**
 * Validates the status of a model.
 * @returns {Promise<NitroModelOperationResponse>} A promise that resolves to an object.
 * If the model is loaded successfully, the object is empty.
 * If the model is not loaded successfully, the object contains an error message.
 */
async function validateModelStatus(): Promise<NitroModelOperationResponse> {
  // Send a GET request to the validation URL.
  // Retry the request up to 3 times if it fails, with a delay of 500 milliseconds between retries.
  return fetchRetry(NITRO_HTTP_VALIDATE_MODEL_URL, {
    method: "GET",
    headers: {
      "Content-Type": "application/json",
    },
    retries: 5,
    retryDelay: 500,
  }).then(async (res: Response) => {
    log(
      `[NITRO]::Debug: Validate model state success with response ${JSON.stringify(
        res,
      )}`,
    );
    // If the response is OK, check model_loaded status.
    if (res.ok) {
      const body = await res.json();
      // If the model is loaded, return an empty object.
      // Otherwise, return an object with an error message.
      if (body.model_loaded) {
        return Promise.resolve({});
      }
    }
    return Promise.resolve({ error: "Validate model status failed" });
  });
}

/**
 * Terminates the Nitro subprocess.
 * @returns A Promise that resolves when the subprocess is terminated successfully, or rejects with an error message if the subprocess fails to terminate.
 */
async function killSubprocess(): Promise<NitroModelOperationResponse> {
  const controller = new AbortController();
  setTimeout(() => controller.abort(), 5000);
  log(`[NITRO]::Debug: Request to kill Nitro`);

  return fetch(NITRO_HTTP_KILL_URL, {
    method: "DELETE",
    signal: controller.signal,
  })
    .then(() => {
      subprocess?.kill();
      subprocess = undefined;
    })
    .then(() => tcpPortUsed.waitUntilFree(PORT, 300, 5000))
    .then(() => log(`[NITRO]::Debug: Nitro process is terminated`))
    .then(() => Promise.resolve({}))
    .catch((err) => ({ error: err }));
}

/**
 * Spawns a Nitro subprocess.
 * @returns A promise that resolves when the Nitro subprocess is started.
 */
function spawnNitroProcess(): Promise<NitroModelOperationResponse> {
  log(`[NITRO]::Debug: Spawning Nitro subprocess...`);

  return new Promise(async (resolve, reject) => {
    const binaryFolder = path.join(__dirname, "..", "bin"); // Current directory by default
    const executableOptions = executableNitroFile(nvidiaConfig);

    const args: string[] = ["1", LOCAL_HOST, PORT.toString()];
    // Execute the binary
    log(
      `[NITRO]::Debug: Spawn nitro at path: ${executableOptions.executablePath}, and args: ${args}`,
    );
    subprocess = spawn(
      executableOptions.executablePath,
      ["1", LOCAL_HOST, PORT.toString()],
      {
        cwd: binaryFolder,
        env: {
          ...process.env,
          CUDA_VISIBLE_DEVICES: executableOptions.cudaVisibleDevices,
        },
      },
    );

    // Handle subprocess output
    subprocess.stdout.on("data", (data: any) => {
      log(`[NITRO]::Debug: ${data}`);
    });

    subprocess.stderr.on("data", (data: any) => {
      log(`[NITRO]::Error: ${data}`);
    });

    subprocess.on("close", (code: any) => {
      log(`[NITRO]::Debug: Nitro exited with code: ${code}`);
      subprocess = undefined;
      reject(`child process exited with code ${code}`);
    });

    tcpPortUsed.waitUntilUsed(PORT, 300, 30000).then(() => {
      log(`[NITRO]::Debug: Nitro is ready`);
      resolve({});
    });
  });
}

/**
 * Get the system resources information
 */
function getResourcesInfo(): Promise<ResourcesInfo> {
  return new Promise(async (resolve) => {
    const cpu = osUtils.cpuCount();
    log(`[NITRO]::CPU informations - ${cpu}`);
    const response: ResourcesInfo = {
      numCpuPhysicalCore: cpu,
      memAvailable: 0,
    };
    resolve(response);
  });
}

export default {
  getNvidiaConfig,
  setNvidiaConfig,
  setLogger,
  runModel,
  stopModel,
  loadLLMModel,
  validateModelStatus,
  chatCompletion,
  killSubprocess,
  updateNvidiaInfo: async () => await updateNvidiaInfo(nvidiaConfig),
  getCurrentNitroProcessInfo: () => getNitroProcessInfo(subprocess),
};
