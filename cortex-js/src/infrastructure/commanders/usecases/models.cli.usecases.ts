import { exit } from 'node:process';
import { ModelsUsecases } from '@/usecases/models/models.usecases';
import { Model, ModelFormat } from '@/domain/models/model.interface';
import { CreateModelDto } from '@/infrastructure/dtos/models/create-model.dto';
import { HuggingFaceRepoData } from '@/domain/models/huggingface.interface';
import { gguf } from '@huggingface/gguf';
import { InquirerService } from 'nest-commander';
import { Inject, Injectable } from '@nestjs/common';

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

  async pullModel(modelId: string, callback: (progress: number) => void) {
    if (modelId.includes('/')) {
      await this.pullHuggingFaceModel(modelId);
    }

    await this.modelsUsecases.downloadModel(modelId, callback);
  }

  private async pullHuggingFaceModel(modelId: string) {
    const data = await this.fetchHuggingFaceRepoData(modelId);
    const { quantization } = await this.inquirerService.inquirer.prompt({
      type: 'list',
      name: 'quantization',
      message: 'Select quantization',
      choices: data.siblings.map((e) => e.quantization).filter((e) => !!e),
    });

    const sibling = data.siblings.filter((e) => !!e.quantization).find(
      (e: any) => e.quantization === quantization,
    );

    if (!sibling) throw 'No expected quantization found';
    const stopWords: string[] = [];
    if (sibling.stopWord) {
      stopWords.push(sibling.stopWord);
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
      settings: {},
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

      if (downloadUrl.endsWith('.gguf')) {
        // getting stop word
        let stopWord = '';
        try {
          const { metadata } = await gguf(downloadUrl);
          // @ts-expect-error "tokenizer.ggml.eos_token_id"
          const index = metadata['tokenizer.ggml.eos_token_id'];
          // @ts-expect-error "tokenizer.ggml.tokens"
          stopWord = metadata['tokenizer.ggml.tokens'][index] ?? '';
          data.siblings[i].stopWord = stopWord;
        } catch (err) {
          console.log('Failed to get stop word: ', err);
        }
      }
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
