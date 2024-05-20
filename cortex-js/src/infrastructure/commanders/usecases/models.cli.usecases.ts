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
  OPEN_CHAT_3_5,
  OPEN_CHAT_3_5_JINJA,
  ZEPHYR,
  ZEPHYR_JINJA,
} from '../prompt-constants';

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
  ) {}

  async startModel(modelId: string): Promise<void> {
    await this.getModelOrStop(modelId);
    await this.modelsUsecases.startModel(modelId);
  }

  async stopModel(modelId: string): Promise<void> {
    await this.getModelOrStop(modelId);
    await this.modelsUsecases.stopModel(modelId);
  }

  async updateModelSettingParams(
    modelId: string,
    settingParams: ModelSettingParams,
  ): Promise<ModelSettingParams> {
    return this.modelsUsecases.updateModelSettingParams(modelId, settingParams);
  }

  async updateModelRuntimeParams(
    modelId: string,
    runtimeParams: ModelRuntimeParams,
  ): Promise<ModelRuntimeParams> {
    return this.modelsUsecases.updateModelRuntimeParams(modelId, runtimeParams);
  }

  private async getModelOrStop(modelId: string): Promise<Model> {
    const model = await this.modelsUsecases.findOne(modelId);
    if (!model) {
      console.debug('Model not found');
      exit(1);
    }
    return model;
  }

  async listAllModels(): Promise<Model[]> {
    return this.modelsUsecases.findAll();
  }

  async getModel(modelId: string): Promise<Model> {
    const model = await this.getModelOrStop(modelId);
    return model;
  }

  async removeModel(modelId: string) {
    await this.getModelOrStop(modelId);
    return this.modelsUsecases.remove(modelId);
  }

  async pullModel(modelId: string) {
    if (modelId.includes('/')) {
      await this.pullHuggingFaceModel(modelId);
    }

    const bar = new SingleBar({}, Presets.shades_classic);
    bar.start(100, 0);
    const callback = (progress: number) => {
      bar.update(progress);
    };
    await this.modelsUsecases.downloadModel(modelId, callback);
  }

  private async pullHuggingFaceModel(modelId: string) {
    const data = await this.fetchHuggingFaceRepoData(modelId);
    const { quantization } = await this.inquirerService.inquirer.prompt({
      type: 'list',
      name: 'quantization',
      message: 'Select quantization',
      choices: data.siblings
        .map((e) => e.quantization)
        .filter((e) => e != null),
    });

    const sibling = data.siblings
      .filter((e) => !!e.quantization)
      .find((e: any) => e.quantization === quantization);

    if (!sibling) throw 'No expected quantization found';

    let stopWord = '';
    let promptTemplate = LLAMA_2;

    try {
      const { metadata } = await gguf(sibling.downloadUrl!);
      // @ts-expect-error "tokenizer.ggml.eos_token_id"
      const index = metadata['tokenizer.ggml.eos_token_id'];
      // @ts-expect-error "tokenizer.ggml.eos_token_id"
      const hfChatTemplate = metadata['tokenizer.chat_template'];
      promptTemplate = this.guessPromptTemplateFromHuggingFace(hfChatTemplate);

      // @ts-expect-error "tokenizer.ggml.tokens"
      stopWord = metadata['tokenizer.ggml.tokens'][index] ?? '';
    } catch (err) {
      console.log('Failed to get stop word: ', err);
    }

    const stopWords: string[] = [];
    if (stopWord.length > 0) {
      stopWords.push(stopWord);
    }

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

      default:
        console.log(
          'Unknown jinja code:',
          jinjaCode,
          'Returning default LLAMA_2',
        );
        return LLAMA_2;
    }
  }

  private async fetchHuggingFaceRepoData(repoId: string) {
    const sanitizedUrl = this.toHuggingFaceUrl(repoId);

    const res = await fetch(sanitizedUrl);
    const response = await res.json();
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

  private toHuggingFaceUrl(repoId: string): string {
    try {
      const url = new URL(`https://huggingface.co/${repoId}`);
      if (url.host !== 'huggingface.co') {
        throw `Invalid Hugging Face repo URL: ${repoId}`;
      }

      const paths = url.pathname.split('/').filter((e) => e.trim().length > 0);
      if (paths.length < 2) {
        throw `Invalid Hugging Face repo URL: ${repoId}`;
      }

      return `${url.origin}/api/models/${paths[0]}/${paths[1]}`;
    } catch (err) {
      if (repoId.startsWith('https')) {
        throw new Error(`Cannot parse url: ${repoId}`);
      }
      throw err;
    }
  }
}
