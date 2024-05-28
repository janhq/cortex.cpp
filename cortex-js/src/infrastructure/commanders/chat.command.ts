import { CommandRunner, SubCommand, Option } from 'nest-commander';
import { ChatCliUsecases } from './usecases/chat.cli.usecases';
import { exit } from 'node:process';

type ChatOptions = {
  threadId?: string;
  message?: string;
  attach: boolean;
};

@SubCommand({ name: 'chat', description: 'Send a chat request to a model' })
export class ChatCommand extends CommandRunner {
  constructor(private readonly chatCliUsecases: ChatCliUsecases) {
    super();
  }

  async run(_input: string[], options: ChatOptions): Promise<void> {
    const modelId = _input[0];
    if (!modelId) {
      console.error('Model ID is required');
      exit(1);
    }

    return this.chatCliUsecases.chat(
      modelId,
      options.threadId,
      options.message,
      options.attach,
    );
  }

  @Option({
    flags: '-t, --thread <thread_id>',
    description: 'Thread Id. If not provided, will create new thread',
  })
  parseThreadId(value: string) {
    return value;
  }

  @Option({
    flags: '-m, --message <message>',
    description: 'Message to send to the model',
    required: true,
  })
  parseModelId(value: string) {
    return value;
  }

  @Option({
    flags: '-a, --attach',
    description: 'Attach to interactive chat session',
    defaultValue: false,
    name: 'attach',
  })
  parseAttach() {
    return true;
  }
}
