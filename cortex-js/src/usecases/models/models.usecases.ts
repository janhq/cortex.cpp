import { CreateModelDto } from '@/infrastructure/dtos/models/create-model.dto';
import { UpdateModelDto } from '@/infrastructure/dtos/models/update-model.dto';
import { ModelEntity } from '@/infrastructure/entities/model.entity';
import { BadRequestException, Inject, Injectable } from '@nestjs/common';
import { Repository } from 'typeorm';
import {
  Model,
  ModelFormat,
  ModelRuntimeParams,
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
import { ExtensionRepository } from '@/domain/repositories/extension.interface';
import { EngineExtension } from '@/domain/abstracts/engine.abstract';
import { HttpService } from '@nestjs/axios';
import { ModelSettingParamsDto } from '@/infrastructure/dtos/models/model-setting-params.dto';
import { normalizeModelId } from '@/infrastructure/commanders/utils/normalize-model-id';
import { firstValueFrom } from 'rxjs';
import { FileManagerService } from '@/file-manager/file-manager.service';
import { AxiosError } from 'axios';
import { TelemetryUsecases } from '../telemetry/telemetry.usecases';
import { TelemetrySource } from '@/domain/telemetry/telemetry.interface';
import { ContextService } from '@/util/context.service';

@Injectable()
export class ModelsUsecases {
  constructor(
    @Inject('MODEL_REPOSITORY')
    private readonly modelRepository: Repository<ModelEntity>,
    private readonly extensionRepository: ExtensionRepository,
    private readonly fileManagerService: FileManagerService,
    private readonly httpService: HttpService,
    private readonly telemetryUseCases: TelemetryUsecases,
    private readonly contextService: ContextService,
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
    this.contextService.set('modelId', id);
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

  async updateModelSettingParams(
    id: string,
    settingParams: ModelSettingParams,
  ): Promise<ModelSettingParams> {
    const model = await this.getModelOrThrow(id);
    const currentSettingParams = model.settings;
    const updateDto: UpdateModelDto = {
      settings: {
        ...currentSettingParams,
        ...settingParams,
      },
    };
    await this.update(id, updateDto);
    return updateDto.settings ?? {};
  }

  async updateModelRuntimeParams(
    id: string,
    runtimeParams: ModelRuntimeParams,
  ): Promise<ModelRuntimeParams> {
    const model = await this.getModelOrThrow(id);
    const currentRuntimeParams = model.parameters;
    const updateDto: UpdateModelDto = {
      parameters: {
        ...currentRuntimeParams,
        ...runtimeParams,
      },
    };
    await this.update(id, updateDto);
    return updateDto.parameters ?? {};
  }

  private async getModelDirectory(): Promise<string> {
    const dataFolderPath = await this.fileManagerService.getDataFolderPath();
    return join(dataFolderPath, 'models');
  }

  async remove(id: string) {
    const modelsContainerDir = await this.getModelDirectory();
    if (!existsSync(modelsContainerDir)) {
      return;
    }

    const modelFolder = join(modelsContainerDir, normalizeModelId(id));

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
    settings?: ModelSettingParamsDto,
  ): Promise<StartModelSuccessDto> {
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
      .loadModel(model, settings)
      .then(() => ({
        message: 'Model loaded successfully',
        modelId,
      }))
      .catch(async (e) => {
        if (e.code === AxiosError.ERR_BAD_REQUEST) {
          return {
            message: 'Model already loaded',
            modelId,
          };
        }
        await this.telemetryUseCases.createCrashReport(
          e,
          TelemetrySource.CORTEX_CPP,
        );
        return {
          message: 'Failed to load model',
          modelId,
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
      .then(() => ({
        message: 'Model is stopped',
        modelId,
      }))
      .catch(async (e) => {
        await this.telemetryUseCases.createCrashReport(
          e,
          TelemetrySource.CORTEX_CPP,
        );
        return {
          message: 'Failed to stop model',
          modelId,
        };
      });
  }

  async downloadModel(modelId: string, callback?: (progress: number) => void) {
    const model = await this.getModelOrThrow(modelId);

    if (model.format === ModelFormat.API) {
      throw new BadRequestException('Cannot download remote model');
    }

    const downloadUrl = model.sources[0].url;
    if (!this.isValidUrl(downloadUrl)) {
      throw new BadRequestException(`Invalid download URL: ${downloadUrl}`);
    }

    const fileName = basename(downloadUrl);
    const modelsContainerDir = await this.getModelDirectory();

    if (!existsSync(modelsContainerDir)) {
      mkdirSync(modelsContainerDir, { recursive: true });
    }

    const modelFolder = join(modelsContainerDir, normalizeModelId(model.id));
    await promises.mkdir(modelFolder, { recursive: true });
    const destination = join(modelFolder, fileName);

    const response = await firstValueFrom(
      this.httpService.get(downloadUrl, {
        responseType: 'stream',
      }),
    );
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
