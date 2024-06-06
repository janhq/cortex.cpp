import { exit } from 'node:process';
import { ModelsUsecases } from '@/usecases/models/models.usecases';
import { Model } from '@/domain/models/model.interface';
import { CreateModelDto } from '@/infrastructure/dtos/models/create-model.dto';
import {
  AllQuantizations,
  HuggingFaceRepoData,
} from '@/domain/models/huggingface.interface';
import { gguf } from '@huggingface/gguf';
import { InquirerService } from 'nest-commander';
import { Inject, Injectable } from '@nestjs/common';
import { Presets, SingleBar } from 'cli-progress';
import {
  LLAMA_2,
  LLAMA_3,
  LLAMA_3_JINJA,
  OPEN_CHAT_3_5,
  OPEN_CHAT_3_5_JINJA,
  ZEPHYR,
  ZEPHYR_JINJA,
} from '../../constants/prompt-constants';
import { ModelTokenizer } from '../types/model-tokenizer.interface';
import { HttpService } from '@nestjs/axios';
import { firstValueFrom } from 'rxjs';
import { StartModelSuccessDto } from '@/infrastructure/dtos/models/start-model-success.dto';
import { UpdateModelDto } from '@/infrastructure/dtos/models/update-model.dto';
import { FileManagerService } from '@/file-manager/file-manager.service';
import { join, basename } from 'path';
import { load } from 'js-yaml';
import { existsSync, readdirSync, readFileSync } from 'fs';
import { isLocalModel, normalizeModelId } from '../utils/normalize-model-id';
import {
  HUGGING_FACE_DOWNLOAD_FILE_MAIN_URL,
  HUGGING_FACE_REPO_MODEL_API_URL,
  HUGGING_FACE_REPO_URL,
  HUGGING_FACE_TREE_REF_URL,
} from '../../constants/huggingface';

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

    await this.pullHuggingFaceModel(modelId);
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

  private async getHFModelTokenizer(
    ggufUrl: string,
  ): Promise<ModelTokenizer | undefined> {
    try {
      const { metadata } = await gguf(ggufUrl);
      // @ts-expect-error "tokenizer.ggml.eos_token_id"
      const index = metadata['tokenizer.ggml.eos_token_id'];
      // @ts-expect-error "tokenizer.ggml.eos_token_id"
      const hfChatTemplate = metadata['tokenizer.chat_template'];
      const promptTemplate =
        this.guessPromptTemplateFromHuggingFace(hfChatTemplate);
      // @ts-expect-error "tokenizer.ggml.tokens"
      const stopWord: string = metadata['tokenizer.ggml.tokens'][index] ?? '';

      return {
        stopWord,
        promptTemplate,
      };
    } catch (err) {
      console.log('Failed to get model metadata:', err);
      return undefined;
    }
  }

  //// PRIVATE METHODS ////

  /**
   * It's to pull model from HuggingFace repository
   * It could be a model from Jan's repo or other authors
   * @param modelId HuggingFace model id. e.g. "janhq/llama-3 or llama3:7b"
   */
  private async pullHuggingFaceModel(modelId: string) {
    let data: HuggingFaceRepoData;
    if (modelId.includes('/'))
      data = await this.fetchHuggingFaceRepoData(modelId);
    else data = await this.fetchJanRepoData(modelId);

    let sibling;

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
      sibling = data.siblings
        .filter((e) => !!e.quantization)
        .find((e: any) => e.quantization === quantization);
    } else {
      sibling = data.siblings.find((e) => e.rfilename.includes('.gguf'));
    }
    if (!sibling) throw 'No expected quantization found';
    const tokenizer = await this.getHFModelTokenizer(sibling.downloadUrl!);

    const promptTemplate = tokenizer?.promptTemplate ?? LLAMA_2;
    const stopWords: string[] = [tokenizer?.stopWord ?? ''];

    const model: CreateModelDto = {
      files: [sibling.downloadUrl ?? ''],
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
      engine: 'cortex.llamacpp',
    };
    if (!(await this.modelsUsecases.findOne(modelId)))
      await this.modelsUsecases.create(model);
  }

  // TODO: move this to somewhere else, should be reused by API as well. Maybe in a separate service / provider?
  private guessPromptTemplateFromHuggingFace(jinjaCode?: string): string {
    if (!jinjaCode) {
      console.log('No jinja code provided. Returning default LLAMA_2');
      return LLAMA_2;
    }

    if (typeof jinjaCode !== 'string') {
      console.log(
        `Invalid jinja code provided (type is ${typeof jinjaCode}). Returning default LLAMA_2`,
      );
      return LLAMA_2;
    }

    switch (jinjaCode) {
      case ZEPHYR_JINJA:
        return ZEPHYR;

      case OPEN_CHAT_3_5_JINJA:
        return OPEN_CHAT_3_5;

      case LLAMA_3_JINJA:
        return LLAMA_3;

      default:
        // console.log(
        //   'Unknown jinja code:',
        //   jinjaCode,
        //   'Returning default LLAMA_2',
        // );
        return LLAMA_2;
    }
  }

  /**
   * Fetch the model data from Jan's repo
   * @param modelId HuggingFace model id. e.g. "llama-3:7b"
   * @returns
   */
  private async fetchJanRepoData(modelId: string) {
    const repo = modelId.split(':')[0];
    const tree = modelId.split(':')[1] ?? 'default';
    const url = this.getRepoModelsUrl(`janhq/${repo}`, tree);
    const res = await fetch(url);
    const response:
      | {
          path: string;
          size: number;
        }[]
      | { error: string } = await res.json();

    if ('error' in response && response.error != null) {
      throw new Error(response.error);
    }

    const data: HuggingFaceRepoData = {
      siblings: Array.isArray(response)
        ? response.map((e) => {
            return {
              rfilename: e.path,
              downloadUrl: HUGGING_FACE_TREE_REF_URL(repo, tree, e.path),
              fileSize: e.size ?? 0,
            };
          })
        : [],
      tags: ['gguf'],
      id: modelId,
      modelId: modelId,
      author: 'janhq',
      sha: '',
      downloads: 0,
      lastModified: '',
      private: false,
      disabled: false,
      gated: false,
      pipeline_tag: 'text-generation',
      cardData: {},
      createdAt: '',
    };

    AllQuantizations.forEach((quantization) => {
      data.siblings.forEach((sibling: any) => {
        if (!sibling.quantization && sibling.rfilename.includes(quantization)) {
          sibling.quantization = quantization;
        }
      });
    });

    data.modelUrl = url;
    return data;
  }

  /**
   * Fetches the model data from HuggingFace API
   * @param repoId HuggingFace model id. e.g. "janhq/llama-3"
   * @returns
   */
  private async fetchHuggingFaceRepoData(repoId: string) {
    const sanitizedUrl = this.getRepoModelsUrl(repoId);

    const res = await firstValueFrom(this.httpService.get(sanitizedUrl));
    const response = res.data;
    if (response['error'] != null) {
      throw new Error(response['error']);
    }

    const data = response as HuggingFaceRepoData;

    if (data.tags.indexOf('gguf') === -1) {
      throw `${repoId} is not supported. Only GGUF models are supported.`;
    }

    // fetching file sizes
    const url = new URL(sanitizedUrl);
    const paths = url.pathname.split('/').filter((e) => e.trim().length > 0);

    for (let i = 0; i < data.siblings.length; i++) {
      const downloadUrl = HUGGING_FACE_DOWNLOAD_FILE_MAIN_URL(
        paths[2],
        paths[3],
        data.siblings[i].rfilename,
      );
      data.siblings[i].downloadUrl = downloadUrl;
    }

    AllQuantizations.forEach((quantization) => {
      data.siblings.forEach((sibling: any) => {
        if (!sibling.quantization && sibling.rfilename.includes(quantization)) {
          sibling.quantization = quantization;
        }
      });
    });

    data.modelUrl = HUGGING_FACE_REPO_URL(paths[2], paths[3]);
    return data;
  }

  private getRepoModelsUrl(repoId: string, tree?: string): string {
    return `${HUGGING_FACE_REPO_MODEL_API_URL(repoId)}${tree ? `/tree/${tree}` : ''}`;
  }

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
