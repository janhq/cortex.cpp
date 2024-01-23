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
  prompt_template?: string;
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
  settings: NitroPromptSetting;
}

/**
 * Logging interface for passing custom logger to nitro-node
 */
interface NitroLogger {
  (message: string, fileName?: string): void;
}