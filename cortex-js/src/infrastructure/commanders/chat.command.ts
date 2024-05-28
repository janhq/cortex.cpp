import { CommandRunner, SubCommand, Option } from 'nest-commander';
import { ChatCliUsecases } from './usecases/chat.cli.usecases';
import { exit } from 'node:process';

type ChatOptions = {
  model?: string;
  threadId?: string;
};

@SubCommand({ name: 'chat', description: 'Start a chat with a model' })
export class ChatCommand extends CommandRunner {
  constructor(private readonly chatCliUsecases: ChatCliUsecases) {
    super();
  }

  async run(_input: string[], option: ChatOptions): Promise<void> {
    const modelId = option.model;
    if (!modelId) {
      console.error('Model ID is required');
      exit(1);
    }

    return this.chatCliUsecases.chat(modelId, option.threadId);
  }

  @Option({
    flags: '-m, --model <model_id>',
    description: 'Model Id to start chat with',
    required: true,
  })
  parseModelId(value: string) {
    return value;
  }

  @Option({
    flags: '-t, --thread <thread_id>',
    description: 'Thread Id. If not provided, will create new thread',
  })
  parseThreadId(value: string) {
    return value;
  }
}
