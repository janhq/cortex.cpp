import { CortexUsecases } from '@/usecases/cortex/cortex.usecases';
import { ModelsUsecases } from '@/usecases/models/models.usecases';
import { CommandRunner, SubCommand } from 'nest-commander';
import { resolve } from 'path';
import { existsSync } from 'fs';
import { ModelSettingParams } from '@/domain/models/model.interface';

@SubCommand({ name: 'start', aliases: ['run'] })
export class StartCommand extends CommandRunner {
  constructor(
    private readonly modelsUsecases: ModelsUsecases,
    private readonly cortexUsecases: CortexUsecases,
  ) {
    super();
  }

  async run(input: string[]): Promise<void> {
    const modelId = input[0];

    if (!modelId) {
      console.log('Model ID is required');
      return;
    }
    return this.startCortex()
      .then(() => this.startModel(modelId))
      .then(console.log)
      .catch(console.error);
  }

  private async startCortex() {
    if (!existsSync(resolve(this.rootDir(), 'cortex-cpp'))) {
      console.log('Please init the cortex by running cortex init command!');
      process.exit(0);
    }
    const host = '127.0.0.1';
    const port = '3928';
    return this.cortexUsecases.startCortex(host, port);
  }

  private async startModel(modelId: string) {
    // TODO: NamH remove these hardcoded value
    const settings: ModelSettingParams = {
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
    return this.modelsUsecases.startModel(modelId, settings);
  }

  rootDir = () => resolve(__dirname, `../../../`);
}
