import { ChatUsecases } from '@/usecases/chat/chat.usecases';
import { CommandRunner, SubCommand } from 'nest-commander';
import { ChatCliUsecases } from './usecases/chat.cli.usecases';

@SubCommand({ name: 'chat' })
export class ChatCommand extends CommandRunner {
  constructor(private readonly chatUsecases: ChatUsecases) {
    super();
  }

  async run(input: string[]): Promise<void> {
    const chatCliService = new ChatCliUsecases(this.chatUsecases);
    return chatCliService.run(input);
  }
}
