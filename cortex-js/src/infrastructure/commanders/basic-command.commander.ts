import { RootCommand, CommandRunner } from 'nest-commander';
import { ModelsUsecases } from 'src/usecases/models/models.usecases';
import { LoadModelDto } from '../dtos/models/load-model.dto';
import { CortexUsecases } from 'src/usecases/cortex/cortex.usecases';
import { CreateModelDto } from '../dtos/models/create-model.dto';
import { ModelFormat } from '@/domain/models/model.interface';
import { spawn } from 'child_process';
import { join } from 'path';

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

@RootCommand({})
export class BasicCommand extends CommandRunner {
  constructor(
    private readonly modelsUsecases: ModelsUsecases,
    private readonly cortexUsecases: CortexUsecases,
  ) {
    super();
  }

  // eslint-disable-next-line @typescript-eslint/no-unused-vars
  async run(input: string[], options?: Record<string, any>): Promise<void> {
    const command = input[0];

    switch (command) {
      case 'pull':
        return this.pullModel(input);

      case 'models':
        this.modelsUsecases.findAll().then((e: any) => console.log(e));
        return;

      case 'serve':
        return this.serve();

      case 'start':
        return this.startCortex();

      case 'load':
        return this.loadModel(input);

      default:
        Promise.reject('Command not supported');
    }
  }

  private async pullModel(input: string[]): Promise<void> {
    if (input.length < 2) {
      return Promise.reject('Model ID is required');
    }
    if (input[1].includes('/')) {
      await this.pullHuggingFaceModel(input[1]);
    }
    this.modelsUsecases.downloadModel({ modelId: input[1] });
  }

  private async startCortex(): Promise<void> {
    const host = '127.0.0.1';
    const port = '3928';
    const result = await this.cortexUsecases.startCortex(host, port);
    console.log(result);
  }

  private async serve(): Promise<void> {
    spawn('node', [join(__dirname, '../../main.js')], {
      detached: false,
      stdio: 'inherit',
    });
  }
  private async loadModel(input: string[]): Promise<void> {
    if (input.length < 2) {
      return Promise.reject('Model ID is required');
    }
    const settings = {
      cpu_threads: 10,
      ctx_len: 2048,
      embedding: false,
      prompt_template:
        '{system_message}\n### Instruction: {prompt}\n### Response:',
      system_prompt: '',
      user_prompt: '\n### Instruction: ',
      ai_prompt: '\n### Response:',
      ngl: 100,
    };
    const loadModelDto: LoadModelDto = { modelId: input[1], settings };
    await this.modelsUsecases
      .loadModel(loadModelDto)
      .then((e) => console.log(e));
  }

  async pullHuggingFaceModel(modelId: string) {
    const data = await this.fetchHuggingFaceRepoData(modelId);

    // TODO: add select options
    const sibling = data.siblings.filter(
      (e: any) => e.quantization == 'Q5_K_M',
    )[0];

    if (!sibling) throw 'No expected quantization found';

    const model: CreateModelDto = {
      sources: [
        {
          url: sibling.downloadUrl,
        },
      ],
      id: modelId,
      name: modelId,
      version: '',
      format: ModelFormat.GGUF,
      description: '',
      settings: {},
      parameters: {},
      metadata: {
        author: data.author,
        size: sibling.fileSize,
        tags: [],
      },
      engine: 'cortex',
    };
    if (!(await this.modelsUsecases.findOne(modelId)))
      await this.modelsUsecases.create(model);
  }

  async fetchHuggingFaceRepoData(repoId: string) {
    const sanitizedUrl = this.toHuggingFaceUrl(repoId);

    const res = await fetch(sanitizedUrl);
    const data = await res.json();
    if (data['error'] != null) {
      throw new Error(data['error']);
    }

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
