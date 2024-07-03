import { Injectable } from '@nestjs/common';
import { ChildProcess, spawn } from 'child_process';
import { delimiter, join } from 'path';
import { CortexOperationSuccessfullyDto } from '@/infrastructure/dtos/cortex/cortex-operation-successfully.dto';
import { HttpService } from '@nestjs/axios';

import { existsSync } from 'node:fs';
import { firstValueFrom } from 'rxjs';
import { FileManagerService } from '@/infrastructure/services/file-manager/file-manager.service';
import {
  CORTEX_CPP_HEALTH_Z_URL,
  CORTEX_CPP_PROCESS_DESTROY_URL,
  CORTEX_JS_STOP_API_SERVER_URL,
} from '@/infrastructure/constants/cortex';
import { createWriteStream, openSync } from 'fs';

@Injectable()
export class CortexUsecases {
  private cortexProcess: ChildProcess | undefined;
  private cortexBinaryName: string = `cortex-cpp${process.platform === 'win32' ? '.exe' : ''}`;

  constructor(
    private readonly httpService: HttpService,
    private readonly fileManagerService: FileManagerService,
  ) {}

  /**
   * Start the Cortex CPP process
   * @param attach
   * @returns
   */
  async startCortex(
    attach: boolean = false,
  ): Promise<CortexOperationSuccessfullyDto> {
    const configs = await this.fileManagerService.getConfig();
    const host = configs.cortexCppHost;
    const port = configs.cortexCppPort;
    if (this.cortexProcess || (await this.healthCheck(host, port))) {
      return {
        message: 'Cortex is already running',
        status: 'success',
      };
    }

    const args: string[] = ['1', host, `${port}`];
    const dataFolderPath = await this.fileManagerService.getDataFolderPath();
    const cortexCppFolderPath = join(dataFolderPath, 'cortex-cpp');
    const cortexCppPath = join(cortexCppFolderPath, this.cortexBinaryName);

    if (!existsSync(cortexCppPath)) {
      throw new Error('The engine is not available, please run "cortex init".');
    }

    const cortexCPPPath = join(
      await this.fileManagerService.getDataFolderPath(),
      'cortex-cpp',
    );

    const writer = openSync(
      join(await this.fileManagerService.getDataFolderPath(), 'cortex.log'),
      'a+',
    );

    // go up one level to get the binary folder, have to also work on windows
    this.cortexProcess = spawn(cortexCppPath, args, {
      detached: !attach,
      cwd: cortexCppFolderPath,
      stdio: [0, writer, writer],
      env: {
        ...process.env,
        CUDA_VISIBLE_DEVICES: '0',
        PATH: (process.env.PATH || '').concat(delimiter, cortexCPPPath),
        LD_LIBRARY_PATH: (process.env.LD_LIBRARY_PATH || '').concat(
          delimiter,
          cortexCPPPath,
        ),
        // // Vulkan - Support 1 device at a time for now
        // ...(executableOptions.vkVisibleDevices?.length > 0 && {
        //   GGML_VULKAN_DEVICE: executableOptions.vkVisibleDevices[0],
        // }),
      },
    });

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

  private healthCheck(host: string, port: number): Promise<boolean> {
    return fetch(CORTEX_CPP_HEALTH_Z_URL(host, port))
      .then((res) => {
        if (res.ok) {
          return true;
        }
        return false;
      })
      .catch(() => false);
  }
}
