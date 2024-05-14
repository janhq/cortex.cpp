import { Injectable } from '@nestjs/common';
import { ChildProcess, spawn } from 'child_process';
import { join } from 'path';
import { CortexOperationSuccessfullyDto } from '@/infrastructure/dtos/cortex/cortex-operation-successfully.dto';
import { HttpService } from '@nestjs/axios';

@Injectable()
export class CortexUsecases {
  private cortexProcess: ChildProcess | undefined;

  constructor(private readonly httpService: HttpService) {}

  async startCortex(
    host: string,
    port: string,
  ): Promise<CortexOperationSuccessfullyDto> {
    if (this.cortexProcess) {
      return {
        message: 'Cortex is already running',
        status: 'success',
      };
    }

    const args: string[] = ['1', host, port];
    // go up one level to get the binary folder, have to also work on windows
    // const binaryFolder = join(binaryPath, '..');
    this.cortexProcess = spawn(
      join(__dirname, '../../../cortex-cpp/cortex-cpp'),
      args,
      {
        detached: false,
        cwd: join(__dirname, '../../../cortex-cpp'),
        stdio: 'inherit',
        env: {
          ...process.env,
          // TODO: NamH need to get below information
          // CUDA_VISIBLE_DEVICES: executableOptions.cudaVisibleDevices,
          // // Vulkan - Support 1 device at a time for now
          // ...(executableOptions.vkVisibleDevices?.length > 0 && {
          //   GGML_VULKAN_DEVICE: executableOptions.vkVisibleDevices[0],
          // }),
        },
      },
    );

    this.registerCortexEvents();

    // Await for the /healthz status ok
    return new Promise<CortexOperationSuccessfullyDto>((resolve, reject) => {
      const interval = setInterval(() => {
        fetch(`http://${host}:${port}/healthz`)
          .then((res) => {
            if (res.ok) {
              clearInterval(interval);
              resolve({
                message: 'Cortex started successfully',
                status: 'success',
              });
            }
          })
          .catch(reject);
      }, 1000);
    });
  }

  async stopCortex(
    host?: string,
    port?: number,
  ): Promise<CortexOperationSuccessfullyDto> {
    try {
      await this.httpService
        .delete(`http://${host}:${port}/processmanager/destroy`)
        .toPromise();
    } catch (err) {
      console.error(err);
    } finally {
      this.cortexProcess?.kill();
      return {
        message: 'Cortex stopped successfully',
        status: 'success',
      };
    }
  }

  private registerCortexEvents() {
    this.cortexProcess?.on('spawn', () => {});

    this.cortexProcess?.on('message', (message) => {
      console.log('Cortex process message', message);
    });

    this.cortexProcess?.on('close', (code, signal) => {
      console.log('Cortex process closed', code, signal);
      console.debug('Cleaning up..');
      this.unregisterCortexEvent();
      this.cortexProcess = undefined;
    });

    this.cortexProcess?.on('error', (err: Error) => {
      console.log('Cortex process error', err);
    });

    this.cortexProcess?.on('exit', (code, signal) => {
      console.log('Cortex process exited', code, signal);
    });
  }

  private unregisterCortexEvent() {
    this.cortexProcess?.removeAllListeners();
  }
}
