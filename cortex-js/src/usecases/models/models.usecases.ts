import { CreateModelDto } from '@/infrastructure/dtos/models/create-model.dto';
import { UpdateModelDto } from '@/infrastructure/dtos/models/update-model.dto';
import { BadRequestException, Injectable } from '@nestjs/common';
import { Model, ModelSettingParams } from '@/domain/models/model.interface';
import { ModelNotFoundException } from '@/infrastructure/exception/model-not-found.exception';
import { join, basename } from 'path';
import {
  promises,
  existsSync,
  mkdirSync,
  rmdirSync,
  createWriteStream,
} from 'fs';
import { StartModelSuccessDto } from '@/infrastructure/dtos/models/start-model-success.dto';
import { ExtensionRepository } from '@/domain/repositories/extension.interface';
import { EngineExtension } from '@/domain/abstracts/engine.abstract';
import { HttpService } from '@nestjs/axios';
import { isLocalModel, normalizeModelId } from '@/utils/normalize-model-id';
import { firstValueFrom } from 'rxjs';
import { FileManagerService } from '@/infrastructure/services/file-manager/file-manager.service';
import { AxiosError } from 'axios';
import { TelemetryUsecases } from '../telemetry/telemetry.usecases';
import { TelemetrySource } from '@/domain/telemetry/telemetry.interface';
import { ModelRepository } from '@/domain/repositories/model.interface';
import { ModelParameterParser } from '@/utils/model-parameter.parser';
import {
  HuggingFaceModelVersion,
  HuggingFaceRepoData,
} from '@/domain/models/huggingface.interface';

import { LLAMA_2 } from '@/infrastructure/constants/prompt-constants';
import { isValidUrl } from '@/utils/urls';
import {
  fetchHuggingFaceRepoData,
  fetchJanRepoData,
  getHFModelMetadata,
} from '@/utils/huggingface';
import { DownloadType } from '@/domain/models/download.interface';
import { EventEmitter2 } from '@nestjs/event-emitter';
import { ModelEvent, ModelId, ModelStatus } from '@/domain/models/model.event';
import { DownloadManagerService } from '@/infrastructure/services/download-manager/download-manager.service';
import { ContextService } from '@/infrastructure/services/context/context.service';

@Injectable()
export class ModelsUsecases {
  constructor(
    private readonly modelRepository: ModelRepository,
    private readonly extensionRepository: ExtensionRepository,
    private readonly fileManagerService: FileManagerService,
    private readonly downloadManagerService: DownloadManagerService,
    private readonly httpService: HttpService,
    private readonly telemetryUseCases: TelemetryUsecases,
    private readonly contextService: ContextService,
    private readonly eventEmitter: EventEmitter2,
  ) {}

  private activeModelStatuses: Record<ModelId, ModelStatus> = {};

  /**
   * Create a new model
   * @param createModelDto Model data
   */
  async create(createModelDto: CreateModelDto) {
    const { model: modelId, owned_by } = createModelDto;
    const model: Model = {
      ...createModelDto,
      id: modelId,
      created: Date.now(),
      object: 'model',
      owned_by: owned_by ?? '',
    };

    await this.modelRepository.create(model);
  }

  /**
   * Find all models
   * @returns Models
   */
  async findAll(): Promise<Model[]> {
    return this.modelRepository.findAll();
  }

  /**
   * Find a model by ID
   * @param model Model ID
   * @returns Model
   */
  async findOne(model: string) {
    this.contextService.set('modelId', model);
    return this.modelRepository.findOne(model);
  }

  /**
   * Get a model by ID or throw an exception
   * @param id Model ID
   * @returns Model
   */
  async getModelOrThrow(id: string): Promise<Model> {
    const model = await this.findOne(id);
    if (!model) {
      throw new ModelNotFoundException(id);
    }
    return model;
  }

  /**
   * Update a model by ID
   * @param id Model ID
   * @param updateModelDto Model data to update
   * @returns Model update status
   */
  update(id: string, updateModelDto: UpdateModelDto) {
    return this.modelRepository.update(id, updateModelDto);
  }

  /**
   * Remove a model by ID
   * @param id Model ID
   * @returns Model removal status
   */
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

  /**
   * Start a model by ID
   * @param modelId Model ID
   * @param settings Model settings
   * @returns Model start status
   */
  async startModel(
    modelId: string,
    settings?: ModelSettingParams,
  ): Promise<StartModelSuccessDto> {
    const model = await this.getModelOrThrow(modelId);
    const engine = (await this.extensionRepository.findOne(
      model!.engine ?? 'cortex.llamacpp',
    )) as EngineExtension | undefined;

    if (!engine) {
      return {
        message: 'No extension handler found for model',
        modelId,
      };
    }

    // update states and emitting event
    this.activeModelStatuses[modelId] = {
      model: modelId,
      status: 'starting',
      metadata: {},
    };
    const modelEvent: ModelEvent = {
      model: modelId,
      event: 'starting',
      metadata: {},
    };
    this.eventEmitter.emit('model.event', modelEvent);

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
          model_path: (model.files as string[])[0],
        }),
      engine: model.engine ?? 'cortex.llamacpp',
      // User / Model settings
      ...parser.parseModelEngineSettings(model),
      ...parser.parseModelEngineSettings(settings ?? {}),
    };

    return engine
      .loadModel(model, loadModelSettings)
      .then(() => {
        this.activeModelStatuses[modelId] = {
          model: modelId,
          status: 'started',
          metadata: {},
        };
        const modelEvent: ModelEvent = {
          model: modelId,
          event: 'started',
          metadata: {},
        };
        this.eventEmitter.emit('model.event', modelEvent);
      })
      .then(() => ({
        message: 'Model loaded successfully',
        modelId,
      }))
      .catch(async (e) => {
        // remove the model from this.activeModelStatus.
        delete this.activeModelStatuses[modelId];
        const modelEvent: ModelEvent = {
          model: modelId,
          event: 'starting-failed',
          metadata: {},
        };
        this.eventEmitter.emit('model.event', modelEvent);
        console.error('Starting model failed', e.code, e.message, e.stack);
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

  /**
   * Stop a running model
   * @param modelId Model Identifier
   * @returns Model stop status
   */
  async stopModel(modelId: string): Promise<StartModelSuccessDto> {
    const model = await this.getModelOrThrow(modelId);
    const engine = (await this.extensionRepository.findOne(
      model!.engine ?? 'cortex.llamacpp',
    )) as EngineExtension | undefined;

    if (!engine) {
      return {
        message: 'No extension handler found for model',
        modelId,
      };
    }

    this.activeModelStatuses[modelId] = {
      model: modelId,
      status: 'stopping',
      metadata: {},
    };
    const modelEvent: ModelEvent = {
      model: modelId,
      event: 'stopping',
      metadata: {},
    };
    this.eventEmitter.emit('model.event', modelEvent);

    return engine
      .unloadModel(modelId)
      .then(() => {
        delete this.activeModelStatuses[modelId];
        const modelEvent: ModelEvent = {
          model: modelId,
          event: 'stopped',
          metadata: {},
        };
        this.eventEmitter.emit('model.event', modelEvent);
      })
      .then(() => ({
        message: 'Model is stopped',
        modelId,
      }))
      .catch(async (e) => {
        const modelEvent: ModelEvent = {
          model: modelId,
          event: 'stopping-failed',
          metadata: {},
        };
        this.eventEmitter.emit('model.event', modelEvent);
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

  /**
   * Download a remote model from HuggingFace or Jan's repo
   * @param modelId Model ID
   * @param callback Callback function to track download progress
   * @returns
   */
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
    if (!isValidUrl(downloadUrl)) {
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

    if (callback != null) {
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
    } else {
      // modelId should be unique
      const downloadId = modelId;

      // inorder to download multiple files, just need to pass more urls and destination to this object
      const urlToDestination: Record<string, string> = {
        [downloadUrl]: destination,
      };

      this.downloadManagerService.submitDownloadRequest(
        downloadId,
        model.name ?? modelId,
        DownloadType.Model,
        urlToDestination,
      );

      return {
        downloadId,
        message: 'Download started',
      };
    }
  }

  /**
   * Abort a download
   * @param downloadId Download ID
   */
  async abortDownloadModel(downloadId: string) {
    this.downloadManagerService.abortDownload(downloadId);
  }

  /**
   * Populate model metadata from a Model repository (HF, Jan...) and download it
   * @param modelId
   */
  async pullModel(modelId: string, callback?: (progress: number) => void) {
    const existingModel = await this.findOne(modelId);
    if (isLocalModel(existingModel?.files)) {
      throw new BadRequestException('Model already exists');
    }

    // Fetch the repo data

    const data = await this.fetchModelMetadata(modelId);
    // Pull the model.yaml
    await this.populateHuggingFaceModel(
      modelId,
      data.siblings.filter((e) => e.quantization != null)[0],
    );

    // Start downloading the model
    await this.downloadModel(modelId, callback);

    const model = await this.findOne(modelId);
    const fileUrl = join(
      await this.fileManagerService.getModelsPath(),
      normalizeModelId(modelId),
      basename((model?.files as string[])[0]),
    );
    await this.update(modelId, {
      files: [fileUrl],
      name: modelId.replace(':default', ''),
    });
  }

  /**
   * It's to pull model from HuggingFace repository
   * It could be a model from Jan's repo or other authors
   * @param modelId HuggingFace model id. e.g. "janhq/llama-3 or llama3:7b"
   */
  async populateHuggingFaceModel(
    modelId: string,
    modelVersion: HuggingFaceModelVersion,
  ) {
    if (!modelVersion) throw 'No expected quantization found';

    const tokenizer = await getHFModelMetadata(modelVersion.downloadUrl!);

    const promptTemplate = tokenizer?.promptTemplate ?? LLAMA_2;
    const stopWords: string[] = [tokenizer?.stopWord ?? ''];

    const model: CreateModelDto = {
      files: [modelVersion.downloadUrl ?? ''],
      model: modelId,
      name: modelId,
      prompt_template: promptTemplate,
      stop: stopWords,

      // Default Inference Params
      stream: true,
      max_tokens: 4098,
      frequency_penalty: 0.7,
      presence_penalty: 0.7,
      temperature: 0.7,
      top_p: 0.7,

      // Default Model Settings
      ctx_len: 4096,
      ngl: 100,
      engine: modelId.includes('onnx') ? 'cortex.onnx' : 'cortex.llamacpp',
    };
    if (!(await this.findOne(modelId))) await this.create(model);
  }

  /**
   * Fetches the model data from HuggingFace
   * @param modelId Model repo id. e.g. llama3, llama3:8b, janhq/llama3
   * @returns Model metadata
   */
  fetchModelMetadata(modelId: string): Promise<HuggingFaceRepoData> {
    if (modelId.includes('/')) return fetchHuggingFaceRepoData(modelId);
    else return fetchJanRepoData(modelId);
  }

  /**
   * Get the current status of the models
   * @returns Model statuses
   */
  getModelStatuses(): Record<ModelId, ModelStatus> {
    return this.activeModelStatuses;
  }
}
