/**
 * The response from the initModel function.
 * @property error - An error message if the model fails to load.
 */
interface NitroModelOperationResponse {
  error?: any;
  modelFile?: string;
}

interface ResourcesInfo {
  numCpuPhysicalCore: number;
  memAvailable: number;
}

/**
 * Setting for prompts when inferencing with Nitro
 */
interface NitroPromptSetting {
  system_prompt?: string;
  ai_prompt?: string;
  user_prompt?: string;
}

/**
 * The available model settings
 */
interface NitroModelSetting extends NitroPromptSetting {
  llama_model_path: string;
  cpu_threads: number;
}

/**
 * The response object for model init operation.
 */
interface NitroModelInitOptions {
  modelFullPath: string;
  promptTemplate?: string;
}

/**
 * Logging interface for passing custom logger to nitro-node
 */
interface NitroLogger {
  (message: string, fileName?: string): void;
}

/**
 * Nvidia settings
 */
interface NitroNvidiaConfig {
  notify: boolean,
  run_mode: "cpu" | "gpu",
  nvidia_driver: {
    exist: boolean,
    version: string,
  },
  cuda: {
    exist: boolean,
    version: string,
  },
  gpus: { id: string, vram: string }[],
  gpu_highest_vram: string,
}