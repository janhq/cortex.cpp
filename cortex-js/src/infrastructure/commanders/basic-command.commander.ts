import { RootCommand, CommandRunner } from 'nest-commander';
import { ModelsUsecases } from 'src/usecases/models/models.usecases';
import { LoadModelDto } from '../dtos/models/load-model.dto';
import { CortexUsecases } from 'src/usecases/cortex/cortex.usecases';
import { PullCommand } from './pull.command';
import { ServeCommand } from './serve.command';
import { InferenceCommand } from './inference.command';

@RootCommand({
  subCommands: [PullCommand, ServeCommand, InferenceCommand],
})
export class BasicCommand extends CommandRunner {
  constructor(
    private readonly modelsUsecases: ModelsUsecases,
    private readonly cortexUsecases: CortexUsecases,
  ) {
    super();
  }

  async run(input: string[]): Promise<void> {
    const command = input[0];

    switch (command) {
      case 'models':
        this.modelsUsecases.findAll().then((e: any) => console.log(e));
        return;

      case 'start':
        return this.startCortex();

      case 'load':
        return this.loadModel(input);

      default:
        console.error(`Command ${command} is not supported`);
        return;
    }
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
    await this.modelsUsecases
      .loadModel(loadModelDto)
      .then((e) => console.log(e));
  }
}
