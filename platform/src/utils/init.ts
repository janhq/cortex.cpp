import { InitOptions } from '@/infrastructure/commanders/types/init-options.interface';
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
  return options;
};