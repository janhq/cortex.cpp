import path from "node:path";
import { getNvidiaConfig } from "./nvidia";

export interface NitroExecutableOptions {
  executablePath: string;
  cudaVisibleDevices: string;
}

/**
 * Find which executable file to run based on the current platform.
 * @returns The name of the executable file to run.
 */
export const executableNitroFile = (
  binaryFolder: string,
  // Default to GPU if CUDA is available when calling
  runMode: "cpu" | "gpu" = getNvidiaConfig().cuda.exist ? "gpu" : "cpu",
): NitroExecutableOptions => {
  const nvidiaSettings = getNvidiaConfig();
  let cudaVisibleDevices = "";
  let binaryName = "nitro";
  /**
   * The binary folder is different for each platform.
   */
  if (process.platform === "win32") {
    /**
     *  For Windows: win-cpu, win-cuda-11-7, win-cuda-12-0
     */
    if (runMode === "cpu") {
      binaryFolder = path.join(binaryFolder, "win-cpu");
    } else {
      if (nvidiaSettings["cuda"].version === "12") {
        binaryFolder = path.join(binaryFolder, "win-cuda-12-0");
      } else {
        binaryFolder = path.join(binaryFolder, "win-cuda-11-7");
      }
      cudaVisibleDevices = nvidiaSettings["gpu_highest_vram"];
    }
    binaryName = "nitro.exe";
  } else if (process.platform === "darwin") {
    /**
     *  For MacOS: mac-arm64 (Silicon), mac-x64 (InteL)
     */
    if (process.arch === "arm64") {
      binaryFolder = path.join(binaryFolder, "mac-arm64");
    } else {
      binaryFolder = path.join(binaryFolder, "mac-x64");
    }
  } else {
    if (runMode === "cpu") {
      binaryFolder = path.join(binaryFolder, "linux-cpu");
    } else {
      if (nvidiaSettings["cuda"].version === "12") {
        binaryFolder = path.join(binaryFolder, "linux-cuda-12-0");
      } else {
        binaryFolder = path.join(binaryFolder, "linux-cuda-11-7");
      }
      cudaVisibleDevices = nvidiaSettings["gpu_highest_vram"];
    }
  }
  return {
    executablePath: path.join(binaryFolder, binaryName),
    cudaVisibleDevices,
  };
};
