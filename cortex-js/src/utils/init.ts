import { InitOptions } from '@/infrastructure/commanders/types/init-options.interface';
import { cpuInfo } from 'cpu-instructions';
import { checkNvidiaGPUExist } from './cuda';

/**
 * Default installation options base on the system
 * @returns
 */
export const defaultInstallationOptions = async (): Promise<InitOptions> => {
  const options: InitOptions = {};

  // Skip check if darwin
  if (process.platform === 'darwin') {
    return options;
  }
  // If Nvidia Driver is installed -> GPU
  options.runMode = (await checkNvidiaGPUExist()) ? 'GPU' : 'CPU';
  options.gpuType = 'Nvidia';
  //CPU Instructions detection
  options.instructions = await detectInstructions();
  return options;
};

const detectInstructions = (): Promise<
  'AVX' | 'AVX2' | 'AVX512' | undefined
> => {
  const cpuInstruction = cpuInfo.cpuInfo()[0] ?? 'AVX';
  console.log(cpuInstruction, 'CPU instructions detected');
  return Promise.resolve(cpuInstruction);
};
