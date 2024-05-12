import { CreateModelDto } from '@/infrastructure/dtos/models/create-model.dto';
import { UpdateModelDto } from '@/infrastructure/dtos/models/update-model.dto';
import { ModelEntity } from '@/infrastructure/entities/model.entity';
import {
  BadRequestException,
  Inject,
  Injectable,
  InternalServerErrorException,
} from '@nestjs/common';
import { Repository } from 'typeorm';
import { Model, ModelFormat } from '@/domain/models/model.interface';
import { ModelNotFoundException } from '@/infrastructure/exception/model-not-found.exception';
import { join, basename } from 'path';
import { promises, createWriteStream, existsSync, mkdir } from 'fs';
import { LoadModelSuccessDto } from '@/infrastructure/dtos/models/load-model-success.dto';
import { LoadModelDto } from '@/infrastructure/dtos/models/load-model.dto';
import { DownloadModelDto } from '@/infrastructure/dtos/models/download-model.dto';
import { ConfigService } from '@nestjs/config';
import { ExtensionRepository } from '@/domain/repositories/extension.interface';
import { EngineExtension } from '@/domain/abstracts/engine.abstract';
import { HttpService } from '@nestjs/axios';

@Injectable()
export class ModelsUsecases {
  constructor(
    @Inject('MODEL_REPOSITORY')
    private readonly modelRepository: Repository<ModelEntity>,
    private readonly extensionRepository: ExtensionRepository,
    private readonly configService: ConfigService,
    private readonly httpService: HttpService,
  ) {}

  async create(createModelDto: CreateModelDto) {
    const model: Model = {
      ...createModelDto,
      object: 'model',
      created: Date.now(),
    };

    await this.modelRepository.insert(model);
  }

  async findAll(): Promise<Model[]> {
    return this.modelRepository.find();
  }

  async findOne(id: string) {
    return this.modelRepository.findOne({
      where: {
        id,
      },
    });
  }

  async getModelOrThrow(id: string): Promise<Model> {
    const model = await this.findOne(id);
    if (!model) {
      throw new ModelNotFoundException(id);
    }
    return model;
  }

  update(id: string, updateModelDto: UpdateModelDto) {
    return this.modelRepository.update(id, updateModelDto);
  }

  async remove(id: string) {
    return this.modelRepository.delete(id);
  }

  async loadModel(loadModelDto: LoadModelDto): Promise<LoadModelSuccessDto> {
    const model = await this.getModelOrThrow(loadModelDto.modelId);
    const extensions = (await this.extensionRepository.findAll()) ?? [];
    const engine = extensions.find((e: any) => e.provider === model?.engine) as
      | EngineExtension
      | undefined;

    if (!engine) {
      return {
        message: 'No extension handler found for model',
        modelId: loadModelDto.modelId,
      };
    }

    return engine
      .loadModel(model)
      .then(() => {
        return {
          message: 'Model loaded successfully',
          modelId: loadModelDto.modelId,
        };
      })
      .catch((err) => {
        console.error(err);
        return {
          message: 'Model failed to load',
          modelId: loadModelDto.modelId,
        };
      });
  }

  async downloadModel(downloadModelDto: DownloadModelDto) {
    const model = await this.getModelOrThrow(downloadModelDto.modelId);

    if (model.format === ModelFormat.API) {
      throw new BadRequestException('Cannot download remote model');
    }

    // TODO: NamH download multiple files

    const downloadUrl = model.sources[0].url;
    if (!this.isValidUrl(downloadUrl)) {
      throw new BadRequestException(`Invalid download URL: ${downloadUrl}`);
    }

    const fileName = basename(downloadUrl);
    const modelsContainerDir =
      this.configService.get<string>('CORTEX_MODELS_DIR') ?? './models';

    if (!existsSync(modelsContainerDir)) {
      mkdir(modelsContainerDir, { recursive: true }, () => {});
    }
    if (!modelsContainerDir) {
      throw new InternalServerErrorException(
        'CORTEX_MODELS_DIR is null. Recheck your environment variables to ensure all required directories are created',
      );
    }

    const modelFolder = join(modelsContainerDir, model.id);
    await promises.mkdir(modelFolder, { recursive: true });
    const destination = join(modelFolder, fileName);
    // eslint-disable-next-line @typescript-eslint/no-var-requires
    const request = require('request');

    const rq = request({ url: model.sources[0].url });

    // eslint-disable-next-line @typescript-eslint/no-var-requires
    const progress = require('request-progress');
    progress(rq, {})
      .on('progress', (progress: any) => {
        process.stdout.clearLine(0);
        process.stdout.cursorTo(0);
        process.stdout.write(
          `Downloading... ${Math.round(progress.percent * 100.0)}%`,
        );
      })
      .on('end', () => {
        process.stdout.clearLine(0);
        process.stdout.cursorTo(0);
        process.stdout.write(`Downloading... 100%\n`);
        process.stdout.write(`Model download complete`);
      })
      .pipe(createWriteStream(destination));

    return {
      message: `Model ${model.id} is being downloaded`,
    };
  }

  async downloadModelProgress(
    downloadModelDto: DownloadModelDto,
    callback: (progress: number) => void,
  ) {
    const model = await this.getModelOrThrow(downloadModelDto.modelId);

    if (model.format === ModelFormat.API) {
      throw new BadRequestException('Cannot download remote model');
    }

    // TODO: NamH download multiple files

    const downloadUrl = model.sources[0].url;
    if (!this.isValidUrl(downloadUrl)) {
      throw new BadRequestException(`Invalid download URL: ${downloadUrl}`);
    }

    const fileName = basename(downloadUrl);
    const modelsContainerDir =
      this.configService.get<string>('CORTEX_MODELS_DIR');

    if (!modelsContainerDir) {
      throw new InternalServerErrorException(
        'CORTEX_MODELS_DIR is null. Recheck your environment variables to ensure all required directories are created',
      );
    }

    const modelFolder = join(modelsContainerDir, model.id);
    await promises.mkdir(modelFolder, { recursive: true });
    const destination = join(modelFolder, fileName);

    const response = await this.httpService
      .get(downloadUrl, {
        responseType: 'stream',
      })
      .toPromise();
    if (!response) {
      throw new Error('Failed to download model');
    }

    return new Promise((resolve, reject) => {
      const writer = createWriteStream(destination);
      let receivedBytes = 0;
      const totalBytes = response.headers['content-length'];

      writer.on('finish', () => {
        resolve(true);
      });

      writer.on('error', (error) => {
        reject(error);
      });

      response.data.on('data', (chunk: any) => {
        receivedBytes += chunk.length;
        callback(Math.floor((receivedBytes / totalBytes) * 100));
      });

      response.data.pipe(writer);
    });
  }

  // TODO: NamH move to a helper or utils
  private isValidUrl(input: string | undefined): boolean {
    if (!input) return false;
    try {
      new URL(input);
      return true;
    } catch (e) {
      return false;
    }
  }
}
