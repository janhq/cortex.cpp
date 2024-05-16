import { CortexUsecases } from '@/usecases/cortex/cortex.usecases';
import { ModelsUsecases } from '@/usecases/models/models.usecases';
import { CommandRunner, SubCommand, Option } from 'nest-commander';
import { exit } from 'node:process';
import { ChatUsecases } from '@/usecases/chat/chat.usecases';
import { ChatCliUsecases } from '../usecases/chat.cli.usecases';

type RunOptions = {
  model?: string;
};

@SubCommand({
  name: 'run',
  description: 'EXPERIMENTAL: Shortcut to start a model and chat',
})
export class RunCommand extends CommandRunner {
  constructor(
    private readonly modelsUsecases: ModelsUsecases,
    private readonly cortexUsecases: CortexUsecases,
    private readonly chatUsecases: ChatUsecases,
  ) {
    super();
  }

  async run(_input: string[], option: RunOptions): Promise<void> {
    const modelId = option.model;
    if (!modelId) {
      console.error('Model ID is required');
      exit(1);
    }

    await this.cortexUsecases.startCortex();
    await this.modelsUsecases.startModel(modelId);
    const chatCliUsecases = new ChatCliUsecases(
      this.chatUsecases,
      this.cortexUsecases,
    );
    await chatCliUsecases.chat(modelId);
  }

  @Option({
    flags: '--model <model_id>',
    description: 'Model Id to start chat with',
  })
  parseModelId(value: string) {
    return value;
  }
}
