import { CreateModelDto } from '@/infrastructure/dtos/models/create-model.dto';
import { UpdateModelDto } from '@/infrastructure/dtos/models/update-model.dto';
import { BadRequestException, Injectable } from '@nestjs/common';
import { Model, ModelSettingParams } from '@/domain/models/model.interface';
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
import { normalizeModelId } from '@/infrastructure/commanders/utils/normalize-model-id';
import { firstValueFrom } from 'rxjs';
import { FileManagerService } from '@/file-manager/file-manager.service';
import { AxiosError } from 'axios';
import { ModelRepository } from '@/domain/repositories/model.interface';
import { ModelParameterParser } from '@/infrastructure/commanders/utils/model-parameter.parser';

@Injectable()
export class ModelsUsecases {
  constructor(
    private readonly modelRepository: ModelRepository,
    private readonly extensionRepository: ExtensionRepository,
    private readonly fileManagerService: FileManagerService,
    private readonly httpService: HttpService,
  ) {}

  async create(createModelDto: CreateModelDto) {
    const model: Model = {
      ...createModelDto,
    };

    await this.modelRepository.create(model);
  }

  async findAll(): Promise<Model[]> {
    return this.modelRepository.findAll();
  }

  async findOne(model: string) {
    return this.modelRepository.findOne(model);
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
    const modelsContainerDir = await this.fileManagerService.getModelsPath();
    if (!existsSync(modelsContainerDir)) {
      return;
    }

    const modelFolder = join(modelsContainerDir, normalizeModelId(id));

    return this.modelRepository
      .remove(id)
      .then(
        () =>
          existsSync(modelFolder) &&
          rmdirSync(modelFolder, { recursive: true }),
      )
      .then(() => {
        return {
          message: 'Model removed successfully',
          modelId: id,
        };
      });
  }

  async startModel(
    modelId: string,
    settings?: ModelSettingParams,
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

    const parser = new ModelParameterParser();
    const loadModelSettings: ModelSettingParams = {
      // Default settings
      ctx_len: 4096,
      ngl: 100,
      //TODO: Utils for model file retrieval
      ...(model?.files &&
        Array.isArray(model.files) &&
        !('llama_model_path' in model) && {
          llama_model_path: (model.files as string[])[0],
        }),
      engine: 'cortex.llamacpp',
      // User / Model settings
      ...parser.parseModelEngineSettings(model),
      ...parser.parseModelEngineSettings(settings ?? {}),
    };

    return engine
      .loadModel(model, loadModelSettings)
      .then(() => ({
        message: 'Model loaded successfully',
        modelId,
      }))
      .catch((e) => ({
        message:
          e.code === AxiosError.ERR_BAD_REQUEST
            ? 'Model already loaded'
            : 'Model failed to load',
        modelId,
      }));
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
      .catch(() => ({
        message: 'Failed to stop model',
        modelId,
      }));
  }

  async downloadModel(modelId: string, callback?: (progress: number) => void) {
    const model = await this.getModelOrThrow(modelId);

    // TODO: We will support splited gguf files in the future
    // Leave it as is for now (first element of the array)
    const downloadUrl = Array.isArray(model.files)
      ? model.files[0]
      : model.files.llama_model_path;

    if (!downloadUrl) {
      throw new BadRequestException('No model URL provided');
    }
    if (!this.isValidUrl(downloadUrl)) {
      throw new BadRequestException(`Invalid download URL: ${downloadUrl}`);
    }

    const fileName = basename(downloadUrl);
    const modelsContainerDir = await this.fileManagerService.getModelsPath();

    if (!existsSync(modelsContainerDir)) {
      mkdirSync(modelsContainerDir, { recursive: true });
    }

    const modelFolder = join(modelsContainerDir, normalizeModelId(model.model));
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
