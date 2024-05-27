import { CortexUsecases } from '@/usecases/cortex/cortex.usecases';
import { ModelsUsecases } from '@/usecases/models/models.usecases';
import { CommandRunner, SubCommand, Option } from 'nest-commander';
import { exit } from 'node:process';
import { ChatCliUsecases } from '../usecases/chat.cli.usecases';
import { defaultCortexCppHost, defaultCortexCppPort } from 'constant';

type RunOptions = {
  threadId?: string;
};

@SubCommand({
  name: 'run',
  description: 'EXPERIMENTAL: Shortcut to start a model and chat',
})
export class RunCommand extends CommandRunner {
  constructor(
    private readonly modelsUsecases: ModelsUsecases,
    private readonly cortexUsecases: CortexUsecases,
    private readonly chatCliUsecases: ChatCliUsecases,
  ) {
    super();
  }

  async run(input: string[], option?: RunOptions): Promise<void> {
    if (input.length === 0) {
      console.error('Model Id is required');
      exit(1);
    }
    const modelId = input[0];

    await this.cortexUsecases.startCortex(
      defaultCortexCppHost,
      defaultCortexCppPort,
      false,
    );
    await this.modelsUsecases.startModel(modelId);
    await this.chatCliUsecases.chat(modelId, option?.threadId);
  }

  @Option({
    flags: '-t, --thread <thread_id>',
    description: 'Thread Id. If not provided, will create new thread',
  })
  parseThreadId(value: string) {
    return value;
  }
}
