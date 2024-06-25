import { exit } from 'node:process';
import { ModelsUsecases } from '@/usecases/models/models.usecases';
import { Model } from '@/domain/models/model.interface';
import { CreateModelDto } from '@/infrastructure/dtos/models/create-model.dto';
import { HuggingFaceRepoData } from '@/domain/models/huggingface.interface';
import { InquirerService } from 'nest-commander';
import { Inject, Injectable } from '@nestjs/common';
import { Presets, SingleBar } from 'cli-progress';
import { LLAMA_2 } from '@/infrastructure/constants/prompt-constants';

import { HttpService } from '@nestjs/axios';
import { StartModelSuccessDto } from '@/infrastructure/dtos/models/start-model-success.dto';
import { UpdateModelDto } from '@/infrastructure/dtos/models/update-model.dto';
import { FileManagerService } from '@/infrastructure/services/file-manager/file-manager.service';
import { join, basename } from 'path';
import { load } from 'js-yaml';
import { existsSync, readdirSync, readFileSync } from 'fs';
import { isLocalModel, normalizeModelId } from '@/utils/normalize-model-id';
import { fetchJanRepoData, getHFModelMetadata } from '@/utils/huggingface';
import { createWriteStream, mkdirSync, promises } from 'node:fs';
import { firstValueFrom } from 'rxjs';
import { Engines } from '../types/engine.interface';

@Injectable()
export class ModelsCliUsecases {
  constructor(
    private readonly modelsUsecases: ModelsUsecases,
    @Inject(InquirerService)
    private readonly inquirerService: InquirerService,
    private readonly httpService: HttpService,
    private readonly fileService: FileManagerService,
  ) {}

  /**
   * Start a model by ID
   * @param modelId
   */
  async startModel(
    modelId: string,
    preset?: string,
  ): Promise<StartModelSuccessDto> {
    const parsedPreset = await this.parsePreset(preset);
    return this.getModelOrStop(modelId)
      .then((model) => ({
        ...model,
        ...parsedPreset,
      }))
      .then((settings) => this.modelsUsecases.startModel(modelId, settings))
      .catch(() => {
        return {
          modelId: modelId,
          message: 'Model not found',
        };
      });
  }

  /**
   * Stop a model by ID
   * @param modelId
   */
  async stopModel(modelId: string): Promise<void> {
    return this.modelsUsecases.stopModel(modelId).then();
  }

  /**
   * Update a model by ID with new data
   */
  async updateModel(modelId: string, toUpdate: UpdateModelDto) {
    return this.modelsUsecases.update(modelId, toUpdate);
  }

  /**
   * Find a model or abort if not exist
   * @param modelId
   * @returns
   */
  async getModelOrStop(modelId: string): Promise<Model> {
    const model = await this.modelsUsecases.findOne(modelId);
    if (!model) {
      console.debug('Model not found');
      exit(1);
    }
    return model;
  }

  /**
   * List all of the models
   * @returns
   */
  async listAllModels(): Promise<Model[]> {
    return this.modelsUsecases.findAll();
  }

  /**
   * Get a model by ID
   * @param modelId
   * @returns
   */
  async getModel(modelId: string): Promise<Model | null> {
    return this.modelsUsecases.findOne(modelId);
  }

  /**
   * Remove a model, this would also delete model files
   * @param modelId
   * @returns
   */
  async removeModel(modelId: string) {
    await this.getModelOrStop(modelId);
    return this.modelsUsecases.remove(modelId);
  }

  /**
   * Pull model from Model repository (HF, Jan...)
   * @param modelId
   */
  async pullModel(modelId: string) {
    const existingModel = await this.modelsUsecases.findOne(modelId);
    if (isLocalModel(existingModel?.files)) {
      console.error('Model already exists');
      process.exit(1);
    }

    if (modelId.includes('onnx') || modelId.includes('tensorrt')) {
      await this.pullEngineModelFiles(modelId);
    } else {
      await this.pullGGUFModel(modelId);
      const bar = new SingleBar({}, Presets.shades_classic);
      bar.start(100, 0);
      const callback = (progress: number) => {
        bar.update(progress);
      };

      try {
        await this.modelsUsecases.downloadModel(modelId, callback);

        const model = await this.modelsUsecases.findOne(modelId);
        const fileUrl = join(
          await this.fileService.getModelsPath(),
          normalizeModelId(modelId),
          basename((model?.files as string[])[0]),
        );
        await this.modelsUsecases.update(modelId, {
          files: [fileUrl],
          name: modelId.replace(':default', ''),
        });
      } catch (err) {
        bar.stop();
        throw err;
      }
    }
  }

  /**
   * It's to pull engine model files from HuggingFace repository
   * @param modelId
   */
  private async pullEngineModelFiles(modelId: string) {
    const modelsContainerDir = await this.fileService.getModelsPath();

    if (!existsSync(modelsContainerDir)) {
      mkdirSync(modelsContainerDir, { recursive: true });
    }

    const modelFolder = join(modelsContainerDir, normalizeModelId(modelId));
    await promises.mkdir(modelFolder, { recursive: true }).catch(() => {});

    const files = (await fetchJanRepoData(modelId)).siblings;
    for (const file of files) {
      console.log(`Downloading ${file.rfilename}`);
      const bar = new SingleBar({}, Presets.shades_classic);
      bar.start(100, 0);
      const response = await firstValueFrom(
        this.httpService.get(file.downloadUrl ?? '', {
          responseType: 'stream',
        }),
      );
      if (!response) {
        throw new Error('Failed to download model');
      }

      await new Promise((resolve, reject) => {
        const writer = createWriteStream(join(modelFolder, file.rfilename));
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
          bar.update(Math.floor((receivedBytes / totalBytes) * 100));
        });

        response.data.pipe(writer);
      });
      bar.stop();
    }

    const model: CreateModelDto = load(
      readFileSync(join(modelFolder, 'model.yml'), 'utf-8'),
    ) as CreateModelDto;
    model.files = [join(modelFolder)];
    model.model = modelId;

    if (!(await this.modelsUsecases.findOne(modelId)))
      await this.modelsUsecases.create(model);

    if (model.engine === Engines.tensorrtLLM) {
      if (process.platform === 'win32')
        console.log(
          'Please ensure that you install MPI and its SDK to use the TensorRT engine, as it also requires the Cuda Toolkit 12.3 to work. Refs:\n- https://github.com/microsoft/Microsoft-MPI/releases/download/v10.1.1/msmpisetup.exe\n- https://github.com/microsoft/Microsoft-MPI/releases/download/v10.1.1/msmpisdk.msi',
        );
      else if (process.platform === 'linux')
        console.log(
          'Please ensure that you install OpenMPI and its SDK to use the TensorRT engine, as it also requires the Cuda Toolkit 12.3 to work.\nYou can install OpenMPI by running "sudo apt update && sudo apt install openmpi-bin libopenmpi-dev"',
        );
    }
  }
  /**
   * It's to pull model from HuggingFace repository
   * It could be a model from Jan's repo or other authors
   * @param modelId HuggingFace model id. e.g. "janhq/llama-3 or llama3:7b"
   */
  private async pullGGUFModel(modelId: string) {
    const data: HuggingFaceRepoData =
      await this.modelsUsecases.fetchModelMetadata(modelId);

    let modelVersion;

    const listChoices = data.siblings
      .filter((e) => e.quantization != null)
      .map((e) => {
        return {
          name: e.quantization,
          value: e.quantization,
        };
      });

    if (listChoices.length > 1) {
      const { quantization } = await this.inquirerService.inquirer.prompt({
        type: 'list',
        name: 'quantization',
        message: 'Select quantization',
        choices: listChoices,
      });
      modelVersion = data.siblings
        .filter((e) => !!e.quantization)
        .find((e: any) => e.quantization === quantization);
    } else {
      modelVersion = data.siblings.find((e) => e.rfilename.includes('.gguf'));
    }

    if (!modelVersion) throw 'No expected quantization found';
    const metadata = await getHFModelMetadata(modelVersion.downloadUrl!);

    const promptTemplate = metadata?.promptTemplate ?? LLAMA_2;
    const stopWords: string[] = [metadata?.stopWord ?? ''];

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
      engine: Engines.llamaCPP,
    };
    if (!(await this.modelsUsecases.findOne(modelId)))
      await this.modelsUsecases.create(model);
  }

  /**
   * Parse preset file
   * @param preset
   * @returns
   */
  private async parsePreset(preset?: string): Promise<object> {
    const presetsFolder = await this.fileService.getPresetsPath();

    if (!existsSync(presetsFolder)) return {};

    const presetFile = readdirSync(presetsFolder).find(
      (file) =>
        file.toLowerCase() === `${preset?.toLowerCase()}.yaml` ||
        file.toLowerCase() === `${preset?.toLocaleLowerCase()}.yml`,
    );
    if (!presetFile) return {};
    const presetPath = join(presetsFolder, presetFile);

    if (!preset || !existsSync(presetPath)) return {};
    return preset
      ? (load(readFileSync(join(presetPath), 'utf-8')) as object)
      : {};
  }
}
