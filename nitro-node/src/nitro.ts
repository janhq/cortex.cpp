import fs from "node:fs";
import path from "node:path";
import stream from "node:stream";
import { ChildProcessWithoutNullStreams, spawn } from "node:child_process";
import tcpPortUsed from "tcp-port-used";
import fetchRT from "fetch-retry";
import { executableNitroFile } from "./execute";
import {
  NitroModelSetting,
  NitroPromptSetting,
  NitroModelOperationResponse,
  NitroModelInitOptions,
  NitroProcessInfo,
  NitroProcessEventHandler,
  NitroProcessStdioHanler,
} from "./types";
import { downloadNitro } from "./scripts";
import { checkMagicBytes, getResourcesInfo } from "./utils";
import { log } from "./logger";
import { updateNvidiaInfo } from "./nvidia";
import { promptTemplateConverter } from "./prompt";
import crossFetch from "cross-fetch";
// Polyfill fetch with retry
const fetchRetry = fetchRT(globalThis.fetch ?? crossFetch);

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

// The supported model format
const SUPPORTED_MODEL_FORMATS = [".gguf"];

// The supported model magic number
const SUPPORTED_MODEL_MAGIC_BYTES = ["GGUF"];

// The subprocess instance for Nitro
let subprocess: ChildProcessWithoutNullStreams | undefined = undefined;
/**
 * Retrieve current nitro process
 */
const getNitroProcessInfo = (subprocess: any): NitroProcessInfo => ({
  isRunning: subprocess != null,
});
const getCurrentNitroProcessInfo = () => getNitroProcessInfo(subprocess);
// Default event handler: do nothing
let processEventHandler: NitroProcessEventHandler = {};
// Default stdio handler: log stdout and stderr
let processStdioHandler: NitroProcessStdioHanler = {
  stdout: (stdout: stream.Readable | null | undefined) => {
    stdout?.on("data", (data: any) => {
      log(`[NITRO]::Debug: ${data}`);
    });
  },
  stderr: (stderr: stream.Readable | null | undefined) => {
    stderr?.on("data", (data: any) => {
      log(`[NITRO]::Error: ${data}`);
    });
  },
};

const registerEventHandler = (handler: NitroProcessEventHandler) => {
  processEventHandler = handler;
};
const registerStdioHandler = (handler: NitroProcessStdioHanler) => {
  processStdioHandler = handler;
};

// The current model file url
let currentModelFile: string = "";
// The current model settings
let currentSettings: NitroModelSetting | undefined = undefined;
// The absolute path to bin directory
let binPath: string = path.join(__dirname, "..", "bin");

/**
 * Get current bin path
 * @returns {string} The bin path
 */
function getBinPath(): string {
  return binPath;
}
/**
 * Set custom bin path
 */
async function setBinPath(customBinPath: string): Promise<void> {
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
 * Initializes the library. Must be called before any other function.
 * This loads the neccesary system information and set some defaults before running model
 */
async function initialize(): Promise<void> {
  // Update nvidia info
  await updateNvidiaInfo();
  log("[NITRO]::Debug: Nitro initialized");
}

/**
 * Stops a Nitro subprocess.
 * @param wrapper - The model wrapper.
 * @returns A Promise that resolves when the subprocess is terminated successfully, or rejects with an error message if the subprocess fails to terminate.
 */
async function stopModel(): Promise<NitroModelOperationResponse> {
  await killSubprocess();
  // Unload settings
  currentSettings = undefined;
  return {};
}

/**
 * Initializes a Nitro subprocess to load a machine learning model.
 * @param modelFullPath - The absolute full path to model directory.
 * @param promptTemplate - The template to use for generating prompts.
 * @returns A Promise that resolves when the model is loaded successfully, or rejects with an error message if the model is not found or fails to load.
 */
async function runModel(
  {
    modelPath,
    promptTemplate,
    ctx_len = 2048,
    ngl = 100,
    cont_batching = false,
    embedding = true,
    cpu_threads,
  }: NitroModelInitOptions,
  runMode?: "cpu" | "gpu",
): Promise<NitroModelOperationResponse> {
  // Download nitro binaries if it's not already downloaded
  await downloadNitro(binPath);
  const files: string[] = fs.readdirSync(modelPath);

  // Look for model file with supported format
  let ggufBinFile = files.find(
    (file) =>
      file === path.basename(modelPath) ||
      SUPPORTED_MODEL_FORMATS.some((ext) => file.toLowerCase().endsWith(ext)),
  );

  // If not found from path and extension, try from magic number
  if (!ggufBinFile) {
    for (const f of files) {
      for (const magicBytes of SUPPORTED_MODEL_MAGIC_BYTES) {
        if (await checkMagicBytes(path.join(modelPath, f), magicBytes)) {
          ggufBinFile = f;
          break;
        }
      }
      if (ggufBinFile) break;
    }
  }

  if (!ggufBinFile) throw new Error("No GGUF model file found");

  currentModelFile = path.join(modelPath, ggufBinFile);

  const nitroResourceProbe = await getResourcesInfo();
  // Convert promptTemplate to system_prompt, user_prompt, ai_prompt
  const prompt: NitroPromptSetting = {};
  if (promptTemplate) {
    Object.assign(prompt, promptTemplateConverter(promptTemplate));
  }

  currentSettings = {
    ...prompt,
    llama_model_path: currentModelFile,
    ctx_len,
    ngl,
    cont_batching,
    embedding,
    // This is critical and requires real system information
    cpu_threads:
      cpu_threads && cpu_threads > 0
        ? cpu_threads
        : Math.max(1, Math.round(nitroResourceProbe.numCpuPhysicalCore / 2)),
  };
  return runNitroAndLoadModel(runMode);
}

/**
 * 1. Spawn Nitro process
 * 2. Load model into Nitro subprocess
 * 3. Validate model status
 * @returns
 */
async function runNitroAndLoadModel(
  runMode?: "cpu" | "gpu",
): Promise<NitroModelOperationResponse> {
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
      await new Promise((resolve) => setTimeout(resolve, 500));
    }
    await spawnNitroProcess(runMode);
    // TODO: Use this response?
    const _loadModelResponse = await loadLLMModel(currentSettings!);
    await validateModelStatus();
    return { modelFile: currentSettings?.llama_model_path };
  } catch (err: any) {
    // TODO: Broadcast error so app could display proper error message
    log(`[NITRO]::Error: ${err}`);
    throw err;
  }
}

/**
 * Loads a LLM model into the Nitro subprocess by sending a HTTP POST request.
 * @returns A Promise that resolves when the model is loaded successfully, or rejects with an error message if the model is not found or fails to load.
 */
async function loadLLMModel(settings: NitroModelSetting): Promise<Response> {
  // The nitro subprocess must be started before loading model
  if (!subprocess) throw Error("Calling loadLLMModel without running nitro");

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
async function validateModelStatus(): Promise<void> {
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
      return;
    }
  }
  throw Error("Validate model status failed");
}

/**
 * Terminates the Nitro subprocess.
 * @returns A Promise that resolves when the subprocess is terminated successfully, or rejects with an error message if the subprocess fails to terminate.
 */
async function killSubprocess(): Promise<void> {
  const controller = new AbortController();
  setTimeout(() => controller.abort(), 5000);
  log(`[NITRO]::Debug: Request to kill Nitro`);

  // Request self-kill if server is running
  if (await tcpPortUsed.check(PORT)) {
    try {
      // FIXME: should use this response?
      const response = await fetch(NITRO_HTTP_KILL_URL, {
        method: "DELETE",
        signal: controller.signal,
      });
    } catch (err: any) {
      // FIXME: Nitro exits without response so fetching will fail
      // Intentionally ignore the error
    }
  }
  // Force kill subprocess
  subprocess?.kill();
  subprocess = undefined;
  await tcpPortUsed.waitUntilFree(PORT, 300, 5000);
  log(`[NITRO]::Debug: Nitro process is terminated`);
  return;
}

/**
 * Spawns a Nitro subprocess.
 * @returns A promise that resolves when the Nitro subprocess is started.
 */
function spawnNitroProcess(runMode?: "cpu" | "gpu"): Promise<void> {
  log(`[NITRO]::Debug: Spawning Nitro subprocess...`);

  return new Promise(async (resolve, reject) => {
    const executableOptions = executableNitroFile(binPath, runMode);

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
    processStdioHandler.stdout(subprocess.stdout);
    processStdioHandler.stderr(subprocess.stderr);
    // Handle events
    let evt: keyof NitroProcessEventHandler;
    for (evt in processEventHandler) {
      subprocess.on(evt, processEventHandler[evt]!);
    }

    subprocess.on("close", (code: number, signal: string) => {
      log(`[NITRO]::Debug: Nitro exited with code: ${code}, signal: ${signal}`);
      subprocess = undefined;
      reject(`child process exited with code ${code}`);
    });

    tcpPortUsed.waitUntilUsed(PORT, 300, 5000).then(() => {
      log(`[NITRO]::Debug: Nitro is ready`);
      resolve();
    });
  });
}

/**
 * Trap for system signal so we can stop nitro process on exit
 */
process.on("SIGTERM", async () => {
  log(`[NITRO]::Debug: Received SIGTERM signal`);
  await killSubprocess();
});

export {
  getCurrentNitroProcessInfo,
  getBinPath,
  setBinPath,
  registerStdioHandler,
  registerEventHandler,
  initialize,
  stopModel,
  runModel,
  loadLLMModel,
  chatCompletion,
  validateModelStatus,
  killSubprocess,
};
