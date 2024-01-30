import os from "node:os";
import fs from "node:fs";
import path from "node:path";
import { ChildProcessWithoutNullStreams, spawn } from "node:child_process";
import tcpPortUsed from "tcp-port-used";
import fetchRT from "fetch-retry";
import osUtils from "os-utils";
import {
  getNitroProcessInfo,
  updateNvidiaInfo as _updateNvidiaInfo,
} from "./nvidia";
import { executableNitroFile } from "./execute";
import {
  NitroNvidiaConfig,
  NitroModelSetting,
  NitroPromptSetting,
  NitroLogger,
  NitroModelOperationResponse,
  NitroModelInitOptions,
  ResourcesInfo,
} from "./types";
import { downloadNitro } from "./scripts";
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
const SUPPORTED_MODEL_FORMATS = [".gguf"];

// The supported model magic number
const SUPPORTED_MODEL_MAGIC_NUMBERS = ["GGUF"];

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
// The absolute path to bin directory
let binPath: string = path.join(__dirname, "..", "bin");

/**
 * Get current bin path
 * @returns {string} The bin path
 */
export function getBinPath(): string {
  return binPath;
}
/**
 * Set custom bin path
 */
export async function setBinPath(customBinPath: string): Promise<void> {
  // Check if the path is a directory
  if (
    fs.existsSync(customBinPath) &&
    fs.statSync(customBinPath).isDirectory()
  ) {
    // If a valid directory, resolve to absolute path and set to binPath
    const resolvedPath = path.resolve(customBinPath);
    binPath = resolvedPath;
  } else {
    throw new Error(`${customBinPath} is not a valid directory!`);
  }
}

/**
 * Get current Nvidia config
 * @returns {NitroNvidiaConfig} A copy of the config object
 * The returned object should be used for reading only
 * Writing to config should be via the function {@setNvidiaConfig}
 */
export function getNvidiaConfig(): NitroNvidiaConfig {
  return Object.assign({}, nvidiaConfig);
}

/**
 * Set custom Nvidia config for running inference over GPU
 * @param {NitroNvidiaConfig} config The new config to apply
 */
export async function setNvidiaConfig(
  config: NitroNvidiaConfig,
): Promise<void> {
  nvidiaConfig = config;
}

/**
 * Set logger before running nitro
 * @param {NitroLogger} logger The logger to use
 */
export async function setLogger(logger: NitroLogger): Promise<void> {
  log = logger;
}

/**
 * Stops a Nitro subprocess.
 * @param wrapper - The model wrapper.
 * @returns A Promise that resolves when the subprocess is terminated successfully, or rejects with an error message if the subprocess fails to terminate.
 */
export function stopModel(): Promise<NitroModelOperationResponse> {
  return killSubprocess();
}

/**
 * Read the magic bytes from a file and check if they match the provided magic bytes
 */
export async function checkMagicBytes(
  filePath: string,
  magicBytes: string,
): Promise<boolean> {
  const desired = Buffer.from(magicBytes);
  const nBytes = desired.byteLength;
  const chunks = [];
  for await (let chunk of fs.createReadStream(filePath, {
    start: 0,
    end: nBytes - 1,
  })) {
    chunks.push(chunk);
  }
  const actual = Buffer.concat(chunks);
  log(`Comparing file's magic bytes <${actual.toString()}> and desired <${desired.toString()}>`);
  return Buffer.compare(actual, desired) === 0;
}

/**
 * Initializes a Nitro subprocess to load a machine learning model.
 * @param modelFullPath - The absolute full path to model directory.
 * @param promptTemplate - The template to use for generating prompts.
 * @returns A Promise that resolves when the model is loaded successfully, or rejects with an error message if the model is not found or fails to load.
 */
export async function runModel({
  modelFullPath,
  promptTemplate,
}: NitroModelInitOptions): Promise<NitroModelOperationResponse> {
  // Download nitro binaries if it's not already downloaded
  await downloadNitro(binPath);
  const files: string[] = fs.readdirSync(modelFullPath);

  // Look for model file with supported format
  let ggufBinFile = files.find(
    (file) =>
      file === path.basename(modelFullPath) ||
      SUPPORTED_MODEL_FORMATS.some((ext) => file.toLowerCase().endsWith(ext)),
  );

  // If not found from path and extension, try from magic number
  if (!ggufBinFile) {
    for (const f of files) {
      for (const magicNum of SUPPORTED_MODEL_MAGIC_NUMBERS) {
        if (await checkMagicBytes(path.join(modelFullPath, f), magicNum)) {
          ggufBinFile = f;
          break;
        }
      }
      if (ggufBinFile) break;
    }
  }

  if (!ggufBinFile) throw new Error("No GGUF model file found");

  currentModelFile = path.join(modelFullPath, ggufBinFile);

  const nitroResourceProbe = await getResourcesInfo();
  // Convert promptTemplate to system_prompt, user_prompt, ai_prompt
  const prompt: NitroPromptSetting = {};
  if (promptTemplate) {
    Object.assign(prompt, promptTemplateConverter(promptTemplate));
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
export async function runNitroAndLoadModel(): Promise<NitroModelOperationResponse> {
  try {
    // Gather system information for CPU physical cores and memory
    await killSubprocess();
    await tcpPortUsed.waitUntilFree(PORT, 300, 5000);
    /**
     * There is a problem with Windows process manager
     * Should wait for awhile to make sure the port is free and subprocess is killed
     * The tested threshold is 500ms
     **/
    if (process.platform === "win32") {
      return await new Promise((resolve) => setTimeout(() => resolve({}), 500));
    }
    const spawnResult = await spawnNitroProcess();
    if (spawnResult.error) {
      return spawnResult;
    }
    // TODO: Use this response?
    const _loadModelResponse = await loadLLMModel(currentSettings);
    const validationResult = await validateModelStatus();
    if (validationResult.error) {
      return validationResult;
    }
    return {};
  } catch (err: any) {
    // TODO: Broadcast error so app could display proper error message
    log(`[NITRO]::Error: ${err}`);
    return { error: err };
  }
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
export async function loadLLMModel(settings: any): Promise<Response> {
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
    return res;
  } catch (err) {
    log(`[NITRO]::Error: Load model failed with error ${err}`);
    throw err;
  }
}

/**
 * Run chat completion by sending a HTTP POST request and stream the response if outStream is specified
 * @param {any} request The request that is then sent to nitro
 * @param {WritableStream} outStream Optional stream that consume the response body
 * @returns {Promise<Response>} A Promise that resolves when the chat completion success, or rejects with an error if the completion fails.
 * @description If outStream is specified, the response body is consumed and cannot be used to reconstruct the data
 */
export async function chatCompletion(
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
  try {
    const response = await fetchRetry(NITRO_HTTP_CHAT_URL, {
      method: "POST",
      headers: {
        "Content-Type": "application/json",
        "Access-Control-Allow-Origin": "*",
        Accept: Boolean(outStream) ? "text/event-stream" : "application/json",
      },
      body: JSON.stringify(request),
      retries: 3,
      retryDelay: 500,
    });
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
  } catch (err) {
    log(`[NITRO]::Error: Chat completion failed with error ${err}`);
    throw err;
  }
}

/**
 * Validates the status of a model.
 * @returns {Promise<NitroModelOperationResponse>} A promise that resolves to an object.
 * If the model is loaded successfully, the object is empty.
 * If the model is not loaded successfully, the object contains an error message.
 */
export async function validateModelStatus(): Promise<NitroModelOperationResponse> {
  // Send a GET request to the validation URL.
  // Retry the request up to 3 times if it fails, with a delay of 500 milliseconds between retries.
  const response = await fetchRetry(NITRO_HTTP_VALIDATE_MODEL_URL, {
    method: "GET",
    headers: {
      "Content-Type": "application/json",
    },
    retries: 5,
    retryDelay: 500,
  });
  log(
    `[NITRO]::Debug: Validate model state success with response ${JSON.stringify(
      response,
    )}`,
  );
  // If the response is OK, check model_loaded status.
  if (response.ok) {
    const body = await response.json();
    // If the model is loaded, return an empty object.
    // Otherwise, return an object with an error message.
    if (body.model_loaded) {
      return {};
    }
  }
  return { error: "Validate model status failed" };
}

/**
 * Terminates the Nitro subprocess.
 * @returns A Promise that resolves when the subprocess is terminated successfully, or rejects with an error message if the subprocess fails to terminate.
 */
export async function killSubprocess(): Promise<NitroModelOperationResponse> {
  const controller = new AbortController();
  setTimeout(() => controller.abort(), 5000);
  log(`[NITRO]::Debug: Request to kill Nitro`);

  try {
    const _response = await fetch(NITRO_HTTP_KILL_URL, {
      method: "DELETE",
      signal: controller.signal,
    });
    subprocess?.kill();
    subprocess = undefined;
    await tcpPortUsed.waitUntilFree(PORT, 300, 5000);
    log(`[NITRO]::Debug: Nitro process is terminated`);
    return {};
  } catch (err) {
    return { error: err };
  }
}

/**
 * Spawns a Nitro subprocess.
 * @returns A promise that resolves when the Nitro subprocess is started.
 */
export function spawnNitroProcess(): Promise<NitroModelOperationResponse> {
  log(`[NITRO]::Debug: Spawning Nitro subprocess...`);

  return new Promise(async (resolve, reject) => {
    const executableOptions = executableNitroFile(nvidiaConfig, binPath);

    const args: string[] = ["1", LOCAL_HOST, PORT.toString()];
    // Execute the binary
    log(
      `[NITRO]::Debug: Spawn nitro at path: ${executableOptions.executablePath}, and args: ${args}`,
    );
    subprocess = spawn(
      executableOptions.executablePath,
      ["1", LOCAL_HOST, PORT.toString()],
      {
        cwd: binPath,
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
export async function getResourcesInfo(): Promise<ResourcesInfo> {
  const cpu = osUtils.cpuCount();
  log(`[NITRO]::CPU informations - ${cpu}`);
  const response: ResourcesInfo = {
    numCpuPhysicalCore: cpu,
    memAvailable: 0,
  };
  return response;
}

export const updateNvidiaInfo = async () =>
  await _updateNvidiaInfo(nvidiaConfig);
export const getCurrentNitroProcessInfo = () => getNitroProcessInfo(subprocess);
