import { ChatUsecases } from '@/usecases/chat/chat.usecases';
import { CommandRunner, SubCommand } from 'nest-commander';
import { CreateChatCompletionDto } from '../dtos/chat/create-chat-completion.dto';
import { ChatCompletionRole } from '@/domain/models/message.interface';
import { stdout } from 'process';
import * as readline from 'node:readline/promises';
import { ChatStreamEvent } from '@/domain/abstracts/oai.abstract';
import { ChatCompletionMessage } from '../dtos/chat/chat-completion-message.dto';

@SubCommand({ name: 'chat' })
export class InferenceCommand extends CommandRunner {
  exitClause = 'exit()';
  userIndicator = '>> ';
  exitMessage = 'Bye!';

  constructor(private readonly chatUsecases: ChatUsecases) {
    super();
  }

  async run(): Promise<void> {
    console.log(`Inorder to exit, type '${this.exitClause}'.`);
    const messages: ChatCompletionMessage[] = [];

    const rl = readline.createInterface({
      input: process.stdin,
      output: process.stdout,
      prompt: this.userIndicator,
    });
    rl.prompt();

    rl.on('close', () => {
      console.log(this.exitMessage);
      process.exit(0);
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
        model: 'TheBloke/TinyLlama-1.1B-Chat-v1.0-GGUF',
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
