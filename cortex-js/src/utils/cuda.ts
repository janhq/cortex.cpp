import { exec } from 'child_process';
import { existsSync } from 'fs';
import { delimiter } from 'path';
import { checkFileExistenceInPaths } from './app-path';

export type GpuSettingInfo = {
  id: string;
  vram: string;
  name: string;
  arch?: string;
};

/**
 * Return the CUDA version installed on the system
 * @returns CUDA Version 11 | 12
 */
export const cudaVersion = async () => {
 
  let filesCuda12: string[];
  let filesCuda11: string[];
  let paths: string[];

  if (process.platform === 'win32') {
    filesCuda12 = ['cublas64_12.dll', 'cudart64_12.dll', 'cublasLt64_12.dll'];
    filesCuda11 = ['cublas64_11.dll', 'cudart64_110.dll', 'cublasLt64_11.dll'];
    paths = process.env.PATH ? process.env.PATH.split(delimiter) : [];
  } else {
    filesCuda12 = ['libcudart.so.12', 'libcublas.so.12', 'libcublasLt.so.12'];
    filesCuda11 = ['libcudart.so.11.0', 'libcublas.so.11', 'libcublasLt.so.11'];
    paths = process.env.LD_LIBRARY_PATH
      ? process.env.LD_LIBRARY_PATH.split(delimiter)
      : [];
    paths.push('/usr/lib/x86_64-linux-gnu/');
  }

  if (
    filesCuda12.every(
      (file) => existsSync(file) || checkFileExistenceInPaths(file, paths),
    )
  )
    return '12';

  if (
    filesCuda11.every(
      (file) => existsSync(file) || checkFileExistenceInPaths(file, paths),
    )
  )
    return '11';

  return undefined; // No CUDA Toolkit found
};

/**
 * Check if an NVIDIA GPU is present
 * @returns GPU driver exist or not
 * TODO: This should be enhanced better
 */
export const checkNvidiaGPUExist = (): Promise<boolean> => {
  return new Promise<boolean>((resolve) => {
    // Execute the nvidia-smi command
    exec('nvidia-smi', (error) => {
      if (error) {
        // If there's an error, it means nvidia-smi is not installed or there's no NVIDIA GPU
        console.log('NVIDIA GPU not detected or nvidia-smi not installed.');
        resolve(false);
      } else {
        // If the command executes successfully, NVIDIA GPU is present
        console.log('NVIDIA GPU detected.');
        resolve(true);
      }
    });
  });
};

export const getCudaVersion = (): Promise<string> => {
  return new Promise<string>((resolve, reject) => {
    // Execute the nvidia-smi command
    exec('nvidia-smi --query-gpu=driver_version --format=csv,noheader', (error, stdout) => {
      if (!error) {
        const firstLine = stdout.split('\n')[0].trim()
        resolve(firstLine);
      } else {
        reject(error);
      }

    });
  });
};

/**
 * Get GPU information from the system
 * @returns GPU information
 */
export const getGpuInfo = async (): Promise<GpuSettingInfo[]> =>
  new Promise((resolve) => {
    exec(
      'nvidia-smi --query-gpu=index,memory.total,name --format=csv,noheader,nounits',
      async (error, stdout) => {
        if (!error) {
          // Get GPU info and gpu has higher memory first
          let highestVram = 0;
          let highestVramId = '0';
          const gpus: GpuSettingInfo[] = stdout
            .trim()
            .split('\n')
            .map((line) => {
              let [id, vram, name] = line.split(', ');
              const arch = getGpuArch(name);
              vram = vram.replace(/\r/g, '');
              if (parseFloat(vram) > highestVram) {
                highestVram = parseFloat(vram);
                highestVramId = id;
              }
              return { id, vram, name, arch };
            });

          resolve(gpus);
        } else {
          resolve([]);
        }
      },
    );
  });

const getGpuArch = (gpuName: string): string => {
  if (!gpuName.toLowerCase().includes('nvidia')) return 'unknown';

  if (gpuName.includes('30')) return 'ampere';
  else if (gpuName.includes('40')) return 'ada';
  else return 'unknown';
};
