import { Injectable } from '@nestjs/common';
import { ChildProcess, spawn } from 'child_process';
import { join } from 'path';
import { CortexOperationSuccessfullyDto } from '@/infrastructure/dtos/cortex/cortex-operation-successfully.dto';
import { HttpService } from '@nestjs/axios';
import { defaultCortexCppHost, defaultCortexCppPort } from 'constant';
import { existsSync } from 'node:fs';
import { firstValueFrom } from 'rxjs';
import { FileManagerService } from '@/file-manager/file-manager.service';

@Injectable()
export class CortexUsecases {
  private cortexProcess: ChildProcess | undefined;
  private cortexBinaryName: string = `cortex-cpp${process.platform === 'win32' ? '.exe' : ''}`;

  constructor(
    private readonly httpService: HttpService,
    private readonly fileManagerService: FileManagerService,
  ) {}

  async startCortex(
    attach: boolean = false,
    host: string = defaultCortexCppHost,
    port: number = defaultCortexCppPort,
  ): Promise<CortexOperationSuccessfullyDto> {
    if (this.cortexProcess) {
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

    // go up one level to get the binary folder, have to also work on windows
    this.cortexProcess = spawn(cortexCppPath, args, {
      detached: !attach,
      cwd: cortexCppFolderPath,
      stdio: attach ? 'inherit' : undefined,
      env: {
        ...process.env,
        CUDA_VISIBLE_DEVICES: '0',
        // // Vulkan - Support 1 device at a time for now
        // ...(executableOptions.vkVisibleDevices?.length > 0 && {
        //   GGML_VULKAN_DEVICE: executableOptions.vkVisibleDevices[0],
        // }),
      },
    });

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
      await firstValueFrom(
        this.httpService.delete(
          `http://${host}:${port}/processmanager/destroy`,
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
}
