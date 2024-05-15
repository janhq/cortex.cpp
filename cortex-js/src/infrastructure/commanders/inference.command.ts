import { ChatUsecases } from '@/usecases/chat/chat.usecases';
import { CommandRunner, SubCommand } from 'nest-commander';
import { CreateChatCompletionDto } from '../dtos/chat/create-chat-completion.dto';
import { ChatCompletionRole } from '@/domain/models/message.interface';
import { exit, stdin, stdout } from 'node:process';
import * as readline from 'node:readline/promises';
import { ChatStreamEvent } from '@/domain/abstracts/oai.abstract';
import { ChatCompletionMessage } from '../dtos/chat/chat-completion-message.dto';

@SubCommand({ name: 'chat' })
export class InferenceCommand extends CommandRunner {
  private exitClause = 'exit()';
  private userIndicator = '>> ';
  private exitMessage = 'Bye!';

  constructor(private readonly chatUsecases: ChatUsecases) {
    super();
  }

  async run(input: string[]): Promise<void> {
    if (input.length == 0) {
      console.error('Please provide a model id.');
      exit(1);
    }

    const modelId = input[0];
    console.log(`Inorder to exit, type '${this.exitClause}'.`);
    const messages: ChatCompletionMessage[] = [];

    const rl = readline.createInterface({
      input: stdin,
      output: stdout,
      prompt: this.userIndicator,
    });
    rl.prompt();

    rl.on('close', () => {
      console.log(this.exitMessage);
      exit(0);
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
