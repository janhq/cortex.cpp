import { writeFileSync, existsSync, readFileSync } from "node:fs";
import { exec } from "node:child_process";
import path from "node:path";


/**
 * Current nitro process
 */
let nitroProcessInfo: NitroProcessInfo | undefined = undefined;

/**
 * Nitro process info
 */
export interface NitroProcessInfo {
  isRunning: boolean
}

/**
 * This will retrive GPU informations and persist settings.json
 * Will be called when the extension is loaded to turn on GPU acceleration if supported
 */
export async function updateNvidiaInfo(nvidiaSettings: NitroNvidiaConfig) {
  if (process.platform !== "darwin") {
    await Promise.all([
      updateNvidiaDriverInfo(nvidiaSettings),
      updateCudaExistence(nvidiaSettings),
      updateGpuInfo(nvidiaSettings),
    ]);
  }
}

/**
 * Retrieve current nitro process
 */
export const getNitroProcessInfo = (subprocess: any): NitroProcessInfo => {
  nitroProcessInfo = {
    isRunning: subprocess != null,
  };
  return nitroProcessInfo;
};

/**
 * Validate nvidia and cuda for linux and windows
 */
export async function updateNvidiaDriverInfo(nvidiaSettings: NitroNvidiaConfig): Promise<void> {
  exec(
    "nvidia-smi --query-gpu=driver_version --format=csv,noheader",
    (error, stdout) => {
      if (!error) {
        const firstLine = stdout.split("\n")[0].trim();
        nvidiaSettings["nvidia_driver"].exist = true;
        nvidiaSettings["nvidia_driver"].version = firstLine;
      } else {
        nvidiaSettings["nvidia_driver"].exist = false;
      }
    }
  );
}

/**
 * Check if file exists in paths
 */
export function checkFileExistenceInPaths(
  file: string,
  paths: string[]
): boolean {
  return paths.some((p) => existsSync(path.join(p, file)));
}

/**
 * Validate cuda for linux and windows
 */
export function updateCudaExistence(nvidiaSettings: NitroNvidiaConfig) {
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
    (file) => existsSync(file) || checkFileExistenceInPaths(file, paths)
  );

  if (!cudaExists) {
    cudaExists = filesCuda11.every(
      (file) => existsSync(file) || checkFileExistenceInPaths(file, paths)
    );
    if (cudaExists) {
      cudaVersion = "11";
    }
  } else {
    cudaVersion = "12";
  }

  nvidiaSettings["cuda"].exist = cudaExists;
  nvidiaSettings["cuda"].version = cudaVersion;
  if (cudaExists) {
    nvidiaSettings.run_mode = "gpu";
  }
}

/**
 * Get GPU information
 */
export async function updateGpuInfo(nvidiaSettings: NitroNvidiaConfig): Promise<void> {
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

        nvidiaSettings["gpus"] = gpus;
        nvidiaSettings["gpu_highest_vram"] = highestVramId;
      } else {
        nvidiaSettings["gpus"] = [];
      }
    }
  );
}
