export interface NitroExecutableOptions {
    executablePath: string;
    cudaVisibleDevices: string;
}
/**
 * Find which executable file to run based on the current platform.
 * @returns The name of the executable file to run.
 */
export declare const executableNitroFile: (nvidiaSettings: NitroNvidiaConfig) => NitroExecutableOptions;
