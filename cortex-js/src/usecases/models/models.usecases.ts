import { CreateModelDto } from '@/infrastructure/dtos/models/create-model.dto';
import { UpdateModelDto } from '@/infrastructure/dtos/models/update-model.dto';
import { ModelEntity } from '@/infrastructure/entities/model.entity';
import { BadRequestException, Inject, Injectable } from '@nestjs/common';
import { Repository } from 'typeorm';
import {
  Model,
  ModelFormat,
  ModelSettingParams,
} from '@/domain/models/model.interface';
import { ModelNotFoundException } from '@/infrastructure/exception/model-not-found.exception';
import { join, basename } from 'path';
import {
  promises,
  createWriteStream,
  existsSync,
  mkdirSync,
  rmdirSync,
} from 'fs';
import { StartModelSuccessDto } from '@/infrastructure/dtos/models/start-model-success.dto';
import { DownloadModelDto } from '@/infrastructure/dtos/models/download-model.dto';
import { ConfigService } from '@nestjs/config';
import { ExtensionRepository } from '@/domain/repositories/extension.interface';
import { EngineExtension } from '@/domain/abstracts/engine.abstract';
import { HttpService } from '@nestjs/axios';
import { ModelSettingParamsDto } from '@/infrastructure/dtos/models/model-setting-params.dto';

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
    const modelsContainerDir =
      this.configService.get<string>('CORTEX_MODELS_DIR') ?? './models';

    if (!existsSync(modelsContainerDir)) {
      return;
    }

    const modelFolder = join(modelsContainerDir, id);

    return this.modelRepository
      .delete(id)
      .then(() => rmdirSync(modelFolder, { recursive: true }))
      .then(() => {
        return {
          message: 'Model removed successfully',
          modelId: id,
        };
      });
  }

  async startModel(
    modelId: string,
    settings: ModelSettingParamsDto,
  ): Promise<StartModelSuccessDto> {
    const model = await this.getModelOrThrow(modelId);
    const extensions = (await this.extensionRepository.findAll()) ?? [];
    const engine = extensions.find((e: any) => e.provider === model?.engine) as
      | EngineExtension
      | undefined;

    if (!engine) {
      return {
        message: 'No extension handler found for model',
        modelId: modelId,
      };
    }

    return engine
      .loadModel(model, settings)
      .then(() => {
        return {
          message: 'Model loaded successfully',
          modelId: modelId,
        };
      })
      .catch((err) => {
        console.error(err);
        return {
          message: 'Model failed to load',
          modelId: modelId,
        };
      });
  }

  async stopModel(modelId: string): Promise<StartModelSuccessDto> {
    const model = await this.getModelOrThrow(modelId);
    const extensions = (await this.extensionRepository.findAll()) ?? [];
    const engine = extensions.find((e: any) => e.provider === model?.engine) as
      | EngineExtension
      | undefined;

    if (!engine) {
      return {
        message: 'No extension handler found for model',
        modelId,
      };
    }

    return engine
      .unloadModel(modelId)
      .then(() => {
        return {
          message: 'Model is stopped',
          modelId,
        };
      })
      .catch((err) => {
        console.error(err);
        return {
          message: 'Failed to stop model',
          modelId,
        };
      });
  }

  async downloadModel(
    downloadModelDto: DownloadModelDto,
    callback?: (progress: number) => void,
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
      this.configService.get<string>('CORTEX_MODELS_DIR') ?? './models';

    if (!existsSync(modelsContainerDir)) {
      mkdirSync(modelsContainerDir, { recursive: true });
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
        callback?.(Math.floor((receivedBytes / totalBytes) * 100));
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
