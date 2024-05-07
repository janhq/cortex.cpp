import { Command, CommandRunner } from 'nest-commander';
import { ModelsUsecases } from 'src/usecases/models/models.usecases';
import { LoadModelDto } from '../dtos/models/load-model.dto';
import { CortexUsecases } from 'src/usecases/cortex/cortex.usecases';

@Command({ name: 'cortex' })
export class BasicCommand extends CommandRunner {
  constructor(
    private readonly modelsUsecases: ModelsUsecases,
    private readonly cortexUsecases: CortexUsecases,
  ) {
    super();
  }

  async run(input: string[], options?: Record<string, any>): Promise<void> {
    const command = input[0];

    switch (command) {
      case 'pull':
        return this.pullModel(input);

      case 'start':
        return this.startCortex();

      case 'load':
        return this.loadModel(input);

      case 'chat':
        return this.inference();

      default:
        Promise.reject('Command not supported');
    }
  }

  private async pullModel(input: string[]): Promise<void> {
    if (input.length < 2) {
      return Promise.reject('Model ID is required');
    }
    this.modelsUsecases.downloadModel({ modelId: input[1] });
  }

  private async startCortex(): Promise<void> {
    const host = '127.0.0.1';
    const port = '3928';
    const result = await this.cortexUsecases.startCortex(host, port);
    console.log(result);
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
    await this.modelsUsecases.loadModel(loadModelDto);
  }
}
