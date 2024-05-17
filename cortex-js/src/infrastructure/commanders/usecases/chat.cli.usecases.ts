import { ChatUsecases } from '@/usecases/chat/chat.usecases';
import { ChatCompletionRole } from '@/domain/models/message.interface';
import { exit, stdin, stdout } from 'node:process';
import * as readline from 'node:readline/promises';
import { ChatStreamEvent } from '@/domain/abstracts/oai.abstract';
import { ChatCompletionMessage } from '@/infrastructure/dtos/chat/chat-completion-message.dto';
import { CreateChatCompletionDto } from '@/infrastructure/dtos/chat/create-chat-completion.dto';
import { CortexUsecases } from '@/usecases/cortex/cortex.usecases';

// TODO: make this class injectable
export class ChatCliUsecases {
  private exitClause = 'exit()';
  private userIndicator = '>> ';
  private exitMessage = 'Bye!';

  constructor(
    private readonly chatUsecases: ChatUsecases,
    private readonly cortexUsecases: CortexUsecases,
  ) {}

  async chat(modelId: string): Promise<void> {
    console.log(`Inorder to exit, type '${this.exitClause}'.`);
    const messages: ChatCompletionMessage[] = [];

    const rl = readline.createInterface({
      input: stdin,
      output: stdout,
      prompt: this.userIndicator,
    });
    rl.prompt();

    rl.on('close', () => {
      this.cortexUsecases.stopCortex().then(() => {
        console.log(this.exitMessage);
        exit(0);
      });
    });

    rl.on('line', (userInput: string) => {
      if (userInput.trim() === this.exitClause) {
        rl.close();
        return;
      }

      messages.push({
        content: userInput,
        role: ChatCompletionRole.User,
      });

      const chatDto: CreateChatCompletionDto = {
        messages,
        model: modelId,
        stream: true,
        max_tokens: 2048,
        stop: [],
        frequency_penalty: 0.7,
        presence_penalty: 0.7,
        temperature: 0.7,
        top_p: 0.7,
      };

      let llmFullResponse = '';
      const writableStream = new WritableStream<ChatStreamEvent>({
        write(chunk) {
          if (chunk.type === 'data') {
            stdout.write(chunk.data ?? '');
            llmFullResponse += chunk.data ?? '';
          } else if (chunk.type === 'error') {
            console.log('Error!!');
          } else {
            messages.push({
              content: llmFullResponse,
              role: ChatCompletionRole.Assistant,
            });
            llmFullResponse = '';
            console.log('\n');
          }
        },
      });

      this.chatUsecases.createChatCompletions(chatDto, {}, writableStream);
    });
  }
}
