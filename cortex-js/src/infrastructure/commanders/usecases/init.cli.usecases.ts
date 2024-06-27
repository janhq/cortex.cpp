import { cpSync, createWriteStream, existsSync, readdirSync, rmSync } from 'fs';
import { delimiter, join } from 'path';
import { HttpService } from '@nestjs/axios';
import { Presets, SingleBar } from 'cli-progress';
import decompress from 'decompress';
import { exit } from 'node:process';
import { InitOptions } from '@commanders/types/init-options.interface';
import { Injectable } from '@nestjs/common';
import { firstValueFrom } from 'rxjs';
import { FileManagerService } from '@/infrastructure/services/file-manager/file-manager.service';
import { rm } from 'fs/promises';
import { exec } from 'child_process';
import { appPath } from '@/utils/app-path';
import {
  CORTEX_ENGINE_RELEASES_URL,
  CORTEX_RELEASES_URL,
  CUDA_DOWNLOAD_URL,
} from '@/infrastructure/constants/cortex';
import { checkNvidiaGPUExist, cudaVersion } from '@/utils/cuda';
import { Engines } from '../types/engine.interface';

@Injectable()
export class InitCliUsecases {
  constructor(
    private readonly httpService: HttpService,
    private readonly fileManagerService: FileManagerService,
  ) {}

  /**
   * Default installation options base on the system
   * @returns
   */
  defaultInstallationOptions = async (): Promise<InitOptions> => {
    let options: InitOptions = {};

    // Skip check if darwin
    if (process.platform === 'darwin') {
      return options;
    }
    // If Nvidia Driver is installed -> GPU
    options.runMode = (await checkNvidiaGPUExist()) ? 'GPU' : 'CPU';
    options.gpuType = 'Nvidia';
    //CPU Instructions detection
    options.instructions = await this.detectInstructions();
    return options;
  };

  /**
   * Install Engine and Dependencies with given options
   * @param engineFileName
   * @param version
   */
  installEngine = async (
    options: InitOptions,
    version: string = 'latest',
    engine: string = 'default',
    force: boolean = true,
  ): Promise<any> => {
    const configs = await this.fileManagerService.getConfig();

    if (configs.initialized && !force) return;

    // Ship Llama.cpp engine by default
    if (
      !existsSync(
        join(
          await this.fileManagerService.getCortexCppEnginePath(),
          'cortex.llamacpp',
        ),
      )
    )
      await this.installLlamaCppEngine(options, version);

    if (engine === Engines.onnx && process.platform !== 'win32') {
      console.error('The ONNX engine does not support this OS yet.');
      process.exit(1);
    }

    if (engine !== 'cortex.llamacpp')
      await this.installAcceleratedEngine('latest', engine);

    configs.initialized = true;
    await this.fileManagerService.writeConfigFile(configs);
  };

  /**
   * Install Llama.cpp engine
   * @param options
   * @param version
   */
  private installLlamaCppEngine = async (
    options: InitOptions,
    version: string = 'latest',
  ) => {
    const engineFileName = this.parseEngineFileName(options);

    const res = await firstValueFrom(
      this.httpService.get(
        CORTEX_RELEASES_URL + `${version === 'latest' ? '/latest' : ''}`,
        {
          headers: {
            'X-GitHub-Api-Version': '2022-11-28',
            Accept: 'application/vnd.github+json',
          },
        },
      ),
    );

    if (!res.data) {
      console.log('Failed to fetch releases');
      exit(1);
    }

    let release = res.data;
    if (Array.isArray(res.data)) {
      release = Array(res.data)[0].find(
        (e) => e.name === version.replace('v', ''),
      );
    }
    const toDownloadAsset = release.assets.find((s: any) =>
      s.name.includes(engineFileName),
    );

    if (!toDownloadAsset) {
      console.log(`Could not find engine ${engineFileName}`);
      exit(1);
    }

    console.log(`Downloading default engine ${engineFileName}`);
    const dataFolderPath = await this.fileManagerService.getDataFolderPath();
    const engineDir = join(dataFolderPath, 'cortex-cpp');
    if (existsSync(engineDir)) rmSync(engineDir, { recursive: true });

    const download = await firstValueFrom(
      this.httpService.get(toDownloadAsset.browser_download_url, {
        responseType: 'stream',
      }),
    );
    if (!download) {
      console.log('Failed to download model');
      process.exit(1);
    }

    const destination = join(dataFolderPath, toDownloadAsset.name);

    await new Promise((resolve, reject) => {
      const writer = createWriteStream(destination);
      let receivedBytes = 0;
      const totalBytes = download.headers['content-length'];

      writer.on('finish', () => {
        bar.stop();
        resolve(true);
      });

      writer.on('error', (error) => {
        bar.stop();
        reject(error);
      });

      const bar = new SingleBar({}, Presets.shades_classic);
      bar.start(100, 0);

      download.data.on('data', (chunk: any) => {
        receivedBytes += chunk.length;
        bar.update(Math.floor((receivedBytes / totalBytes) * 100));
      });

      download.data.pipe(writer);
    });

    try {
      await decompress(destination, join(dataFolderPath));
    } catch (e) {
      console.error('Error decompressing file', e);
      exit(1);
    }

    await rm(destination, { force: true });

    // If the user selected GPU mode and Nvidia GPU, install CUDA Toolkit dependencies
    if (options.runMode === 'GPU' && !(await cudaVersion())) {
      await this.installCudaToolkitDependency(options.cudaVersion);
    }
  };

  /**
   * Parse the engine file name based on the options
   * Please check cortex-cpp release artifacts for the available engine files
   * @param options
   * @returns
   */
  private parseEngineFileName = (options?: InitOptions) => {
    const platform =
      process.platform === 'win32'
        ? 'windows'
        : process.platform === 'darwin'
          ? 'mac'
          : process.platform;
    const arch = process.arch === 'arm64' ? process.arch : 'amd64';
    const cudaVersion =
      options?.runMode === 'GPU'
        ? options.gpuType === 'Nvidia'
          ? '-cuda-' + (options.cudaVersion === '11' ? '11-7' : '12-0')
          : '-vulkan'
        : '';
    const instructions = options?.instructions
      ? `-${options.instructions}`
      : '';
    const engineName = `${platform}-${arch}${instructions.toLowerCase()}${cudaVersion}`;
    return `${engineName}.tar.gz`;
  };

  /**
   * Install CUDA Toolkit dependency (dll/so files)
   * @param options
   */
  private installCudaToolkitDependency = async (cudaVersion?: string) => {
    const platform = process.platform === 'win32' ? 'windows' : 'linux';

    const dataFolderPath = await this.fileManagerService.getDataFolderPath();
    const url = CUDA_DOWNLOAD_URL.replace(
      '<version>',
      cudaVersion === '11' ? '11.7' : '12.3',
    ).replace('<platform>', platform);
    const destination = join(dataFolderPath, 'cuda-toolkit.tar.gz');

    console.log('Downloading CUDA Toolkit dependency...');
    const download = await firstValueFrom(
      this.httpService.get(url, {
        responseType: 'stream',
      }),
    );

    if (!download) {
      console.log('Failed to download dependency');
      process.exit(1);
    }

    await new Promise((resolve, reject) => {
      const writer = createWriteStream(destination);
      let receivedBytes = 0;
      const totalBytes = download.headers['content-length'];

      writer.on('finish', () => {
        bar.stop();
        resolve(true);
      });

      writer.on('error', (error) => {
        bar.stop();
        reject(error);
      });

      const bar = new SingleBar({}, Presets.shades_classic);
      bar.start(100, 0);

      download.data.on('data', (chunk: any) => {
        receivedBytes += chunk.length;
        bar.update(Math.floor((receivedBytes / totalBytes) * 100));
      });

      download.data.pipe(writer);
    });

    try {
      await decompress(destination, join(dataFolderPath, 'cortex-cpp'));
    } catch (e) {
      console.log(e);
      exit(1);
    }
    await rm(destination, { force: true });
  };

  private detectInstructions = (): Promise<
    'AVX' | 'AVX2' | 'AVX512' | undefined
  > => {
    return new Promise<'AVX' | 'AVX2' | 'AVX512' | undefined>((res) => {
      // Execute the cpuinfo command

      exec(
        join(
          appPath,
          `bin/cpuinfo${process.platform !== 'linux' ? '.exe' : ''}`,
        ),
        (error, stdout) => {
          if (error) {
            // If there's an error, it means lscpu is not installed
            console.log('CPUInfo is not installed.');
            res('AVX');
          } else {
            // If the command executes successfully, parse the output to detect CPU instructions
            if (stdout.includes('"AVX512": "true"')) {
              console.log('AVX-512 instructions detected.');
              res('AVX512');
            } else if ('"AVX2": "true"') {
              console.log('AVX2 instructions detected.');
              res('AVX2');
            } else {
              console.log('AVXs instructions detected.');
              res('AVX');
            }
          }
        },
      );
    });
  };

  /**
   * Download and install accelerated engine
   * @param version
   * @param engineFileName
   */
  private async installAcceleratedEngine(
    version: string = 'latest',
    engine: string = Engines.onnx,
  ) {
    const res = await firstValueFrom(
      this.httpService.get(
        CORTEX_ENGINE_RELEASES_URL(engine) +
          `${version === 'latest' ? '/latest' : ''}`,
        {
          headers: {
            'X-GitHub-Api-Version': '2022-11-28',
            Accept: 'application/vnd.github+json',
          },
        },
      ),
    );

    if (!res?.data) {
      console.log('Failed to fetch releases');
      exit(1);
    }

    let release = res?.data;
    if (Array.isArray(res?.data)) {
      release = Array(res?.data)[0].find(
        (e) => e.name === version.replace('v', ''),
      );
    }
    const toDownloadAsset = release.assets.find((s: any) =>
      s.name.includes(process.platform === 'win32' ? 'windows' : 'linux'),
    );

    if (!toDownloadAsset) {
      console.log(
        `Could not find engine file for platform ${process.platform}`,
      );
      exit(1);
    }

    console.log(`Downloading engine file ${toDownloadAsset.name}`);
    const dataFolderPath = await this.fileManagerService.getDataFolderPath();
    const engineDir = join(dataFolderPath, 'cortex-cpp');

    const download = await firstValueFrom(
      this.httpService.get(toDownloadAsset.browser_download_url, {
        responseType: 'stream',
      }),
    );
    if (!download) {
      console.log('Failed to download model');
      process.exit(1);
    }

    const destination = join(dataFolderPath, toDownloadAsset.name);

    await new Promise((resolve, reject) => {
      const writer = createWriteStream(destination);
      let receivedBytes = 0;
      const totalBytes = download.headers['content-length'];

      writer.on('finish', () => {
        bar.stop();
        resolve(true);
      });

      writer.on('error', (error) => {
        bar.stop();
        reject(error);
      });

      const bar = new SingleBar({}, Presets.shades_classic);
      bar.start(100, 0);

      download.data.on('data', (chunk: any) => {
        receivedBytes += chunk.length;
        bar.update(Math.floor((receivedBytes / totalBytes) * 100));
      });

      download.data.pipe(writer);
    });

    try {
      await decompress(destination, join(engineDir, 'engines'));
    } catch (e) {
      console.error('Error decompressing file', e);
      exit(1);
    }
    await rm(destination, { force: true });

    // Copy the additional files to the cortex-cpp directory
    for (const file of readdirSync(join(engineDir, 'engines', engine))) {
      if (file !== 'engine.dll') {
        await cpSync(
          join(engineDir, 'engines', engine, file),
          join(engineDir, file),
        );
      }
    }
  }
}
