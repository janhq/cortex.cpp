import { exit } from 'node:process';
import { ModelsUsecases } from '@/usecases/models/models.usecases';
import {
  Model,
  ModelFormat,
  ModelRuntimeParams,
  ModelSettingParams,
} from '@/domain/models/model.interface';
import { CreateModelDto } from '@/infrastructure/dtos/models/create-model.dto';
import { HuggingFaceRepoData } from '@/domain/models/huggingface.interface';
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
} from '../prompt-constants';
import { ModelTokenizer } from '../types/model-tokenizer.interface';
import { HttpService } from '@nestjs/axios';
import { firstValueFrom } from 'rxjs';
import { StartModelSuccessDto } from '@/infrastructure/dtos/models/start-model-success.dto';
import { FileManagerService } from '@/file-manager/file-manager.service';
import { join } from 'path';
import { load } from 'js-yaml';
import { readFileSync } from 'node:fs';

const AllQuantizations = [
  'Q3_K_S',
  'Q3_K_M',
  'Q3_K_L',
  'Q4_K_S',
  'Q4_K_M',
  'Q5_K_S',
  'Q5_K_M',
  'Q4_0',
  'Q4_1',
  'Q5_0',
  'Q5_1',
  'IQ2_XXS',
  'IQ2_XS',
  'Q2_K',
  'Q2_K_S',
  'Q6_K',
  'Q8_0',
  'F16',
  'F32',
  'COPY',
];

@Injectable()
export class ModelsCliUsecases {
  constructor(
    private readonly modelsUsecases: ModelsUsecases,
    @Inject(InquirerService)
    private readonly inquirerService: InquirerService,
    private readonly httpService: HttpService,
    private readonly fileManagerService: FileManagerService,
  ) {}

  /**
   * Start a model by ID
   * @param modelId
   */
  async startModel(
    modelId: string,
    template?: string,
  ): Promise<StartModelSuccessDto> {
    const settings: any = template
      ? load(
          readFileSync(
            join(
              await this.fileManagerService.getDataFolderPath(),
              'templates',
              `${template}.yaml`,
            ),
            'utf-8',
          ),
        )
      : {};

    return this.getModelOrStop(modelId)
      .then((model) => ({ ...model.settings, ...settings }))
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
    return this.getModelOrStop(modelId)
      .then(() => this.modelsUsecases.stopModel(modelId))
      .then();
  }

  /**
   * Update model's settings. E.g. ngl, prompt_template, etc.
   * @param modelId
   * @param settingParams
   * @returns
   */
  async updateModelSettingParams(
    modelId: string,
    settingParams: ModelSettingParams,
  ): Promise<ModelSettingParams> {
    return this.modelsUsecases.updateModelSettingParams(modelId, settingParams);
  }

  /**
   * Update model's runtime parameters. E.g. max_tokens, temperature, etc.
   * @param modelId
   * @param runtimeParams
   * @returns
   */
  async updateModelRuntimeParams(
    modelId: string,
    runtimeParams: ModelRuntimeParams,
  ): Promise<ModelRuntimeParams> {
    return this.modelsUsecases.updateModelRuntimeParams(modelId, runtimeParams);
  }

  /**
   * Find a model or abort if not exist
   * @param modelId
   * @returns
   */
  private async getModelOrStop(modelId: string): Promise<Model> {
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
  async getModel(modelId: string): Promise<Model> {
    const model = await this.getModelOrStop(modelId);
    return model;
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

  async pullModelWithExactUrl(modelId: string, url: string, fileSize: number) {
    const tokenizer = await this.getHFModelTokenizer(url);
    const promptTemplate = tokenizer?.promptTemplate ?? LLAMA_2;
    const stopWords: string[] = [tokenizer?.stopWord ?? ''];

    const model: CreateModelDto = {
      sources: [
        {
          url: url,
        },
      ],
      id: modelId,
      name: modelId,
      version: '',
      format: ModelFormat.GGUF,
      description: '',
      settings: {
        prompt_template: promptTemplate,
      },
      parameters: {
        stop: stopWords,
      },
      metadata: {
        author: 'janhq',
        size: fileSize,
        tags: [],
      },
      engine: 'cortex',
    };
    if (!(await this.modelsUsecases.findOne(modelId))) {
      await this.modelsUsecases.create(model);
    }

    const bar = new SingleBar({}, Presets.shades_classic);
    bar.start(100, 0);
    const callback = (progress: number) => {
      bar.update(progress);
    };
    await this.modelsUsecases.downloadModel(modelId, callback);
  }

  /**
   * Pull model from Model repository (HF, Jan...)
   * @param modelId
   */
  async pullModel(modelId: string) {
    if (modelId.includes('/') || modelId.includes(':')) {
      await this.pullHuggingFaceModel(modelId);
    }
    const bar = new SingleBar({}, Presets.shades_classic);
    bar.start(100, 0);
    const callback = (progress: number) => {
      bar.update(progress);
    };
    await this.modelsUsecases.downloadModel(modelId, callback);
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
      sources: [
        {
          url: sibling?.downloadUrl ?? '',
        },
      ],
      id: modelId,
      name: modelId,
      version: '',
      format: ModelFormat.GGUF,
      description: '',
      settings: {
        prompt_template: promptTemplate,
        llama_model_path: sibling.rfilename,
      },
      parameters: {
        stop: stopWords,
      },
      metadata: {
        author: data.author,
        size: sibling.fileSize ?? 0,
        tags: [],
      },
      engine: 'cortex',
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
    const tree = modelId.split(':')[1];
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
              downloadUrl: `https://huggingface.co/janhq/${repo}/resolve/${tree}/${e.path}`,
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
      const downloadUrl = `https://huggingface.co/${paths[2]}/${paths[3]}/resolve/main/${data.siblings[i].rfilename}`;
      data.siblings[i].downloadUrl = downloadUrl;
    }

    AllQuantizations.forEach((quantization) => {
      data.siblings.forEach((sibling: any) => {
        if (!sibling.quantization && sibling.rfilename.includes(quantization)) {
          sibling.quantization = quantization;
        }
      });
    });

    data.modelUrl = `https://huggingface.co/${paths[2]}/${paths[3]}`;
    return data;
  }

  private getRepoModelsUrl(repoId: string, tree?: string): string {
    return `https://huggingface.co/api/models/${repoId}${tree ? `/tree/${tree}` : ''}`;
  }
}
