import { Injectable } from '@nestjs/common';
import { ChildProcess, spawn } from 'child_process';
import { join, delimiter } from 'path';
import { CortexOperationSuccessfullyDto } from '@/infrastructure/dtos/cortex/cortex-operation-successfully.dto';
import { HttpService } from '@nestjs/axios';
import {
  defaultCortexCppHost,
  defaultCortexCppPort,
} from '@/infrastructure/constants/cortex';
import { existsSync } from 'node:fs';
import { firstValueFrom } from 'rxjs';
import { FileManagerService } from '@/infrastructure/services/file-manager/file-manager.service';
import {
  CORTEX_CPP_HEALTH_Z_URL,
  CORTEX_CPP_PROCESS_DESTROY_URL,
} from '@/infrastructure/constants/cortex';

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
    await this.addAdditionalDependencies();

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
    });
  }

  async stopCortex(): Promise<CortexOperationSuccessfullyDto> {
    try {
      await firstValueFrom(
        this.httpService.delete(CORTEX_CPP_PROCESS_DESTROY_URL()),
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

  private async addAdditionalDependencies() {
    const cortexCPPPath = join(
      await this.fileManagerService.getDataFolderPath(),
      'cortex-cpp',
    );
    const additionalLlamaCppPath = delimiter.concat(
      join(cortexCPPPath, 'cortex.llamacpp'),
    );
    const additionalTensortLLMCppPath = delimiter.concat(
      join(cortexCPPPath, 'cortex.tensorrt-llm'),
    );
    const additionalPaths = delimiter.concat(
      additionalLlamaCppPath,
      additionalTensortLLMCppPath,
    );
    // Set the updated PATH
    process.env.PATH = (process.env.PATH || '').concat(additionalPaths);
    process.env.LD_LIBRARY_PATH = (process.env.LD_LIBRARY_PATH || '').concat(
      additionalPaths,
    );
  }
}
