import { createWriteStream, existsSync, rmSync } from 'fs';
import { CommandRunner, SubCommand, InquirerService } from 'nest-commander';
import { resolve } from 'path';
import { HttpService } from '@nestjs/axios';
import { Presets, SingleBar } from 'cli-progress';
import decompress from 'decompress';
import { exit } from 'node:process';

@SubCommand({ name: 'init', aliases: ['setup'] })
export class InitCommand extends CommandRunner {
  CORTEX_RELEASES_URL = 'https://api.github.com/repos/janhq/cortex/releases';

  constructor(
    private readonly httpService: HttpService,
    private readonly inquirerService: InquirerService,
  ) {
    super();
  }

  async run(input: string[], options?: any): Promise<void> {
    options = await this.inquirerService.ask('create-init-questions', options);
    const version = input[0] ?? 'latest';

    await this.download(this.parseEngineFileName(options), version);
  }

  download = async (
    engineFileName: string,
    version: string = 'latest',
  ): Promise<any> => {
    const res = await this.httpService
      .get(
        this.CORTEX_RELEASES_URL + `${version === 'latest' ? '/latest' : ''}`,
        {
          headers: {
            'X-GitHub-Api-Version': '2022-11-28',
            Accept: 'application/vnd.github+json',
          },
        },
      )
      .toPromise();

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
      s.name.includes(engineFileName),
    );

    if (!toDownloadAsset) {
      console.log(`Could not find engine file ${engineFileName}`);
      exit(1);
    }

    console.log(`Downloading engine file ${engineFileName}`);
    const engineDir = resolve(this.rootDir(), 'cortex-cpp');
    if (existsSync(engineDir)) rmSync(engineDir, { recursive: true });

    const download = await this.httpService
      .get(toDownloadAsset.browser_download_url, {
        responseType: 'stream',
      })
      .toPromise();
    if (!download) {
      throw new Error('Failed to download model');
    }

    const destination = resolve(this.rootDir(), toDownloadAsset.name);

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
      await decompress(
        resolve(this.rootDir(), destination),
        resolve(this.rootDir()),
      );
    } catch (e) {
      console.log(e);
      exit(1);
    }
    exit(0);
  };

  parseEngineFileName = (options: {
    runMode?: 'CPU' | 'GPU';
    gpuType?: 'Nvidia' | 'Others (Vulkan)';
    instructions?: 'AVX' | 'AVX2' | 'AVX512' | undefined;
    cudaVersion?: '11' | '12';
  }) => {
    const platform =
      process.platform === 'win32'
        ? 'windows'
        : process.platform === 'darwin'
          ? 'mac'
          : process.platform;
    const arch = process.arch === 'arm64' ? process.arch : 'amd64';
    const cudaVersion =
      options.runMode === 'GPU'
        ? options.gpuType === 'Nvidia'
          ? '-cuda-' + (options.cudaVersion === '11' ? '11-7' : '12-0')
          : '-vulkan'
        : '';
    const instructions = options.instructions ? `-${options.instructions}` : '';
    const engineName = `${platform}-${arch}${instructions.toLowerCase()}${cudaVersion}`;
    return `${engineName}.tar.gz`;
  };

  rootDir = () => resolve(__dirname, `../../../`);
}
