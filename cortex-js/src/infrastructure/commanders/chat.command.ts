import { ChatUsecases } from '@/usecases/chat/chat.usecases';
import { CommandRunner, SubCommand } from 'nest-commander';
import { ChatCliUsecases } from './usecases/chat.cli.usecases';
import { CortexUsecases } from '@/usecases/cortex/cortex.usecases';

@SubCommand({ name: 'chat' })
export class ChatCommand extends CommandRunner {
  constructor(
    private readonly chatUsecases: ChatUsecases,
    private readonly cortexUsecases: CortexUsecases,
  ) {
    super();
  }

  async run(input: string[]): Promise<void> {
    const modelId = input[0];
    if (!modelId) {
      console.error('Model ID is required');
      process.exit(1);
    }

    const chatCliUsecases = new ChatCliUsecases(
      this.chatUsecases,
      this.cortexUsecases,
    );
    return chatCliUsecases.chat(modelId);
  }
}
