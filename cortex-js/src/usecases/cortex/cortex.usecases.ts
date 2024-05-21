import { Injectable } from '@nestjs/common';
import { ChildProcess, spawn } from 'child_process';
import { join } from 'path';
import { CortexOperationSuccessfullyDto } from '@/infrastructure/dtos/cortex/cortex-operation-successfully.dto';
import { HttpService } from '@nestjs/axios';
import { defaultCortexCppHost, defaultCortexCppPort } from 'constant';
import { existsSync } from 'node:fs';

@Injectable()
export class CortexUsecases {
  private cortexProcess: ChildProcess | undefined;

  constructor(private readonly httpService: HttpService) {}

  async startCortex(
    host: string = defaultCortexCppHost,
    port: number = defaultCortexCppPort,
    verbose: boolean = false,
  ): Promise<CortexOperationSuccessfullyDto> {
    if (this.cortexProcess) {
      return {
        message: 'Cortex is already running',
        status: 'success',
      };
    }

    const args: string[] = ['1', host, `${port}`];
    const cortexCppPath = join(
      __dirname,
      '../../../cortex-cpp/cortex-cpp' +
        `${process.platform === 'win32' ? '.exe' : ''}`,
    );

    if (!existsSync(cortexCppPath)) {
      throw new Error('Cortex binary not found');
    }

    // go up one level to get the binary folder, have to also work on windows
    this.cortexProcess = spawn(cortexCppPath, args, {
      detached: false,
      cwd: join(__dirname, '../../../cortex-cpp'),
      stdio: verbose ? 'inherit' : undefined,
      env: {
        ...process.env,
        CUDA_VISIBLE_DEVICES: '0',
        // // Vulkan - Support 1 device at a time for now
        // ...(executableOptions.vkVisibleDevices?.length > 0 && {
        //   GGML_VULKAN_DEVICE: executableOptions.vkVisibleDevices[0],
        // }),
      },
    });

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
      console.error(err.response.data);
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
