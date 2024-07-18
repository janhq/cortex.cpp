import { HttpStatus, Injectable } from '@nestjs/common';
import { ChildProcess, fork } from 'child_process';
import { delimiter, join } from 'path';
import { CortexOperationSuccessfullyDto } from '@/infrastructure/dtos/cortex/cortex-operation-successfully.dto';
import { HttpService } from '@nestjs/axios';

import { firstValueFrom } from 'rxjs';
import { FileManagerService } from '@/infrastructure/services/file-manager/file-manager.service';
import {
  CORTEX_CPP_HEALTH_Z_URL,
  CORTEX_CPP_PROCESS_DESTROY_URL,
  CORTEX_JS_HEALTH_URL,
  CORTEX_JS_STOP_API_SERVER_URL,
  defaultCortexJsHost,
  defaultCortexJsPort,
} from '@/infrastructure/constants/cortex';
import { openSync } from 'fs';

@Injectable()
export class CortexUsecases {
  private cortexProcess: ChildProcess | undefined;

  constructor(
    private readonly httpService: HttpService,
    private readonly fileManagerService: FileManagerService,
  ) {}

  /**
   * Start the Cortex CPP process
   * @param attach
   * @returns
   */
  async startCortex(): Promise<CortexOperationSuccessfullyDto> {
    const configs = await this.fileManagerService.getConfig();
    const host = configs.cortexCppHost;
    const port = configs.cortexCppPort;
    if (this.cortexProcess || (await this.healthCheck(host, port))) {
      return {
        message: 'Cortex is already running',
        status: 'success',
      };
    }

    const engineDir = await this.fileManagerService.getCortexCppEnginePath();
    const dataFolderPath = await this.fileManagerService.getDataFolderPath();

    const writer = openSync(await this.fileManagerService.getLogPath(), 'a+');
    // go up one level to get the binary folder, have to also work on windows
    this.cortexProcess = fork(join(__dirname, './../../utils/cortex-cpp'), [], {
      detached: true,
      cwd: dataFolderPath,
      stdio: [0, writer, writer, 'ipc'],
      env: {
        ...process.env,
        CUDA_VISIBLE_DEVICES: '0',
        ENGINE_PATH: dataFolderPath,
        PATH: (process.env.PATH || '').concat(delimiter, engineDir),
        LD_LIBRARY_PATH: (process.env.LD_LIBRARY_PATH || '').concat(
          delimiter,
          engineDir,
        ),
        // // Vulkan - Support 1 device at a time for now
        // ...(executableOptions.vkVisibleDevices?.length > 0 && {
        //   GGML_VULKAN_DEVICE: executableOptions.vkVisibleDevices[0],
        // }),
      },
    });
    this.cortexProcess.unref();

    // Await for the /healthz status ok
    return new Promise<CortexOperationSuccessfullyDto>((resolve, reject) => {
      const interval = setInterval(() => {
        this.healthCheck(host, port)
          .then(() => {
            clearInterval(interval);
            resolve({
              message: 'Cortex started successfully',
              status: 'success',
            });
          })
          .catch(reject);
      }, 1000);
    }).then((res) => {
      this.fileManagerService.writeConfigFile({
        ...configs,
        cortexCppHost: host,
        cortexCppPort: port,
      });
      return res;
    });
  }

  /**
   * Stop the Cortex CPP process
   */
  async stopCortex(): Promise<CortexOperationSuccessfullyDto> {
    const configs = await this.fileManagerService.getConfig();
    try {
      await firstValueFrom(
        this.httpService.delete(
          CORTEX_CPP_PROCESS_DESTROY_URL(
            configs.cortexCppHost,
            configs.cortexCppPort,
          ),
        ),
      );
    } catch (err) {
      console.error(err.response.data);
    } finally {
      this.cortexProcess?.kill();
      return {
        message: 'Cortex stopped successfully',
        status: 'success',
      };
    }
  }

  /**
   * Stop the API server
   * @returns
   */
  async stopServe(): Promise<void> {
    return fetch(CORTEX_JS_STOP_API_SERVER_URL(), {
      method: 'DELETE',
    })
      .then(() => {})
      .catch(() => {});
  }

  /**
   * Check whether the Cortex CPP is healthy
   * @param host
   * @param port
   * @returns
   */
  healthCheck(host: string, port: number): Promise<boolean> {
    return fetch(CORTEX_CPP_HEALTH_Z_URL(host, port))
      .then((res) => {
        if (res.ok) {
          return true;
        }
        return false;
      })
      .catch(() => false);
  }

  /**
   * start the API server in detached mode
   */
  async startServerDetached(host: string, port: number) {
    const writer = openSync(await this.fileManagerService.getLogPath(), 'a+');
    console.log(
      'Starting server in detached mode...',
      join(__dirname, './../../main.js'),
    );
    const server = fork(join(__dirname, './../../main.js'), [], {
      detached: true,
      stdio: ['ignore', writer, writer, 'ipc'],
      env: {
        CORTEX_JS_HOST: host,
        CORTEX_JS_PORT: port.toString(),
      },
    });
    server.disconnect(); // closes the IPC channel
    server.unref();
    // Await for the /healthz status ok
    return new Promise<boolean>((resolve, reject) => {
      const TIMEOUT = 20000;
      const timeout = setTimeout(() => {
        clearInterval(interval);
        clearTimeout(timeout);
        reject();
      }, TIMEOUT);
      const interval = setInterval(() => {
        this.isAPIServerOnline()
          .then((result) => {
            console.log('Checking API server status...', result);
            if (result) {
              clearInterval(interval);
              clearTimeout(timeout);
              resolve(true);
            }
          })
          .catch(reject);
      }, 1000);
    });
  }
  /**
   * Check if the Cortex API server is online
   * @returns
   */

  async isAPIServerOnline(): Promise<boolean> {
    const {
      apiServerHost: configApiServerHost,
      apiServerPort: configApiServerPort,
    } = await this.fileManagerService.getConfig();

    // for backward compatibility, we didn't have the apiServerHost and apiServerPort in the config file in the past
    const apiServerHost = configApiServerHost || defaultCortexJsHost;
    const apiServerPort = configApiServerPort || defaultCortexJsPort;
    return firstValueFrom(
      this.httpService.get(CORTEX_JS_HEALTH_URL(apiServerHost, apiServerPort)),
    )
      .then((res) => res.status === HttpStatus.OK)
      .catch(() => false);
  }

  async updateApiServerConfig(host: string, port: number) {
    const config = await this.fileManagerService.getConfig();
    await this.fileManagerService.writeConfigFile({
      ...config,
      cortexCppHost: host,
      cortexCppPort: port,
    });
  }
}
