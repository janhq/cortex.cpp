import { existsSync } from "node:fs";
import { exec } from "node:child_process";
import { NitroNvidiaConfig } from "./types";
import path from "node:path";

// The default config for using Nvidia GPU
const NVIDIA_DEFAULT_CONFIG: NitroNvidiaConfig = {
  notify: true,
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

// The Nvidia info config for checking for CUDA support on the system
let nvidiaConfig: NitroNvidiaConfig = NVIDIA_DEFAULT_CONFIG;

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
 * This will retrieve GPU informations
 * Should be called when the library is loaded to turn on GPU acceleration if supported
 */
export async function updateNvidiaInfo() {
  if (process.platform !== "darwin") {
    await Promise.all([
      updateNvidiaDriverInfo(),
      updateCudaExistence(),
      updateGpuInfo(),
    ]);
  }
}

/**
 * Validate nvidia and cuda for linux and windows
 */
async function updateNvidiaDriverInfo(): Promise<void> {
  exec(
    "nvidia-smi --query-gpu=driver_version --format=csv,noheader",
    (error, stdout) => {
      if (!error) {
        const firstLine = stdout.split("\n")[0].trim();
        nvidiaConfig["nvidia_driver"].exist = true;
        nvidiaConfig["nvidia_driver"].version = firstLine;
      } else {
        nvidiaConfig["nvidia_driver"].exist = false;
      }
    },
  );
}

/**
 * Check if file exists in paths
 */
export function checkFileExistenceInPaths(
  file: string,
  paths: string[],
): boolean {
  return paths.some((p) => existsSync(path.join(p, file)));
}

/**
 * Validate cuda for linux and windows
 */
function updateCudaExistence() {
  let filesCuda12: string[];
  let filesCuda11: string[];
  let paths: string[];
  let cudaVersion: string = "";

  if (process.platform === "win32") {
    filesCuda12 = ["cublas64_12.dll", "cudart64_12.dll", "cublasLt64_12.dll"];
    filesCuda11 = ["cublas64_11.dll", "cudart64_11.dll", "cublasLt64_11.dll"];
    paths = process.env.PATH ? process.env.PATH.split(path.delimiter) : [];
  } else {
    filesCuda12 = ["libcudart.so.12", "libcublas.so.12", "libcublasLt.so.12"];
    filesCuda11 = ["libcudart.so.11.0", "libcublas.so.11", "libcublasLt.so.11"];
    paths = process.env.LD_LIBRARY_PATH
      ? process.env.LD_LIBRARY_PATH.split(path.delimiter)
      : [];
    paths.push("/usr/lib/x86_64-linux-gnu/");
  }

  let cudaExists = filesCuda12.every(
    (file) => existsSync(file) || checkFileExistenceInPaths(file, paths),
  );

  if (!cudaExists) {
    cudaExists = filesCuda11.every(
      (file) => existsSync(file) || checkFileExistenceInPaths(file, paths),
    );
    if (cudaExists) {
      cudaVersion = "11";
    }
  } else {
    cudaVersion = "12";
  }

  nvidiaConfig["cuda"].exist = cudaExists;
  nvidiaConfig["cuda"].version = cudaVersion;
}

/**
 * Get GPU information
 */
async function updateGpuInfo(): Promise<void> {
  exec(
    "nvidia-smi --query-gpu=index,memory.total --format=csv,noheader,nounits",
    (error, stdout) => {
      if (!error) {
        // Get GPU info and gpu has higher memory first
        let highestVram = 0;
        let highestVramId = "0";
        let gpus = stdout
          .trim()
          .split("\n")
          .map((line) => {
            let [id, vram] = line.split(", ");
            vram = vram.replace(/\r/g, "");
            if (parseFloat(vram) > highestVram) {
              highestVram = parseFloat(vram);
              highestVramId = id;
            }
            return { id, vram };
          });

        nvidiaConfig["gpus"] = gpus;
        nvidiaConfig["gpu_highest_vram"] = highestVramId;
      } else {
        nvidiaConfig["gpus"] = [];
      }
    },
  );
}
