/**
 * Nitro process info
 */
export interface NitroProcessInfo {
    isRunning: boolean;
}
/**
 * This will retrive GPU informations and persist settings.json
 * Will be called when the extension is loaded to turn on GPU acceleration if supported
 */
export declare function updateNvidiaInfo(nvidiaSettings: NitroNvidiaConfig): Promise<void>;
/**
 * Retrieve current nitro process
 */
export declare const getNitroProcessInfo: (subprocess: any) => NitroProcessInfo;
/**
 * Validate nvidia and cuda for linux and windows
 */
export declare function updateNvidiaDriverInfo(nvidiaSettings: NitroNvidiaConfig): Promise<void>;
/**
 * Check if file exists in paths
 */
export declare function checkFileExistenceInPaths(file: string, paths: string[]): boolean;
/**
 * Validate cuda for linux and windows
 */
export declare function updateCudaExistence(nvidiaSettings: NitroNvidiaConfig): void;
/**
 * Get GPU information
 */
export declare function updateGpuInfo(nvidiaSettings: NitroNvidiaConfig): Promise<void>;
