import { ModelsUsecases } from '@/usecases/models/models.usecases';
import { CommandRunner, SubCommand } from 'nest-commander';
import { CreateModelDto } from '../dtos/models/create-model.dto';
import { ModelFormat } from '@/domain/models/model.interface';
import { Presets, SingleBar } from 'cli-progress';

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

@SubCommand({ name: 'pull', aliases: ['download'] })
export class PullCommand extends CommandRunner {
  constructor(private readonly modelsUsecases: ModelsUsecases) {
    super();
  }

  async run(input: string[]): Promise<void> {
    if (input.length < 1) {
      return Promise.reject('Model ID is required');
    }

    const modelId = input[0];
    if (modelId.includes('/')) {
      await this.pullHuggingFaceModel(modelId);
    }

    const bar = new SingleBar({}, Presets.shades_classic);
    bar.start(100, 0);
    await this.modelsUsecases.downloadModel({ modelId }, (progress) => {
      bar.update(progress);
    });
    console.log('\nDownload complete!');
    process.exit(0);
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

  private async fetchHuggingFaceRepoData(repoId: string) {
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
