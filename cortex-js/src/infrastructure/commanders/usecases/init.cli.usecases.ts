import { createWriteStream, existsSync, rmSync } from 'fs';
import { resolve, delimiter, join } from 'path';
import { HttpService } from '@nestjs/axios';
import { Presets, SingleBar } from 'cli-progress';
import decompress from 'decompress';
import { exit } from 'node:process';
import { InitOptions } from '../types/init-options.interface';
import { Injectable } from '@nestjs/common';

@Injectable()
export class InitCliUsecases {
    CORTEX_RELEASES_URL = 'https://api.github.com/repos/janhq/cortex/releases';
    CUDA_DOWNLOAD_URL = 'https://catalog.jan.ai/dist/cuda-dependencies/<version>/<platform>/cuda.tar.gz'

    constructor(
        private readonly httpService: HttpService,
    ) {
    }

    installEngine = async (
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
            console.log('Failed to download model');
            process.exit(1)
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
    };

    parseEngineFileName = (options: InitOptions) => {
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

    cudaVersion = async () => {
        let filesCuda12: string[]
        let filesCuda11: string[]
        let paths: string[]

        if (process.platform === 'win32') {
            filesCuda12 = ['cublas64_12.dll', 'cudart64_12.dll', 'cublasLt64_12.dll']
            filesCuda11 = ['cublas64_11.dll', 'cudart64_110.dll', 'cublasLt64_11.dll']
            paths = process.env.PATH ? process.env.PATH.split(delimiter) : []
        } else {
            filesCuda12 = ['libcudart.so.12', 'libcublas.so.12', 'libcublasLt.so.12']
            filesCuda11 = ['libcudart.so.11.0', 'libcublas.so.11', 'libcublasLt.so.11']
            paths = process.env.LD_LIBRARY_PATH
                ? process.env.LD_LIBRARY_PATH.split(delimiter)
                : []
            paths.push('/usr/lib/x86_64-linux-gnu/')
        }

        if (filesCuda12.every(
            (file) => existsSync(file) || this.checkFileExistenceInPaths(file, paths)
        )) return '12'


        if (filesCuda11.every(
            (file) => existsSync(file) || this.checkFileExistenceInPaths(file, paths)
        )) return '11'

        return undefined // No CUDA Toolkit found
    }

    checkFileExistenceInPaths = (file: string, paths: string[]): boolean => {
        return paths.some((p) => existsSync(join(p, file)))
    }

    installCudaToolkitDependency = async (options: InitOptions) => {
        const platform = process.platform === 'win32' ? 'windows' : 'linux'

        const url = this.CUDA_DOWNLOAD_URL
            .replace('<version>', options.cudaVersion === '11' ? '11.7' : '12.0')
            .replace('<platform>', platform)
        const destination = resolve(this.rootDir(), 'cuda-toolkit.tar.gz');

        const download = await this.httpService
            .get(url, {
                responseType: 'stream',
            })
            .toPromise();

        if (!download) {
            console.log('Failed to download dependency');
            process.exit(1)
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
            await decompress(
                resolve(this.rootDir(), destination),
                resolve(this.rootDir(), 'cortex-cpp'),
            );
        } catch (e) {
            console.log(e);
            exit(1);
        }
    }
}