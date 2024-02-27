import { ChildProcessWithoutNullStreams } from "node:child_process";
import net from "node:net";
import stream from "node:stream";

/**
 * The response from the initModel function.
 * @property error - An error message if the model fails to load.
 */
export interface NitroModelOperationResponse {
  modelFile?: string;
}

export interface ResourcesInfo {
  numCpuPhysicalCore: number;
  memAvailable: number;
}

/**
 * Nitro process info
 */
export interface NitroProcessInfo {
  isRunning: boolean;
}

export interface NitroProcessEventHandler {
  close?: (code: number, signal: string) => void;
  disconnect?: () => void;
  error?: (e: Error) => void;
  exit?: (code: number, signal: string) => void;
  message?: (
    message: object,
    sendHandle: net.Socket | net.Server | undefined,
  ) => void;
  spawn?: () => void;
}

export interface NitroProcessStdioHanler {
  stdout: (_: stream.Readable | null | undefined) => void;
  stderr: (_: stream.Readable | null | undefined) => void;
}

/**
 * Setting for prompts when inferencing with Nitro
 */
export interface NitroPromptSetting {
  system_prompt?: string;
  ai_prompt?: string;
  user_prompt?: string;
}

/**
 * The available model settings
 */
export interface NitroModelSetting extends NitroPromptSetting {
  llama_model_path: string;
  ctx_len: number;
  ngl: number;
  cont_batching: boolean;
  embedding: boolean;
  cpu_threads: number;
}

/**
 * The parameters for model init operation.
 */
export interface NitroModelInitOptions {
  modelPath: string;
  promptTemplate?: string;
  ctx_len?: number;
  ngl?: number;
  cont_batching?: boolean;
  embedding?: boolean;
  cpu_threads?: number;
}

/**
 * Logging interface for passing custom logger to nitro-node
 */
export interface NitroLogger {
  (message: string, fileName?: string): void;
}

/**
 * Nvidia settings
 */
export interface NitroNvidiaConfig {
  notify: boolean;
  nvidia_driver: {
    exist: boolean;
    version: string;
  };
  cuda: {
    exist: boolean;
    version: string;
  };
  gpus: { id: string; vram: string }[];
  gpu_highest_vram: string;
}