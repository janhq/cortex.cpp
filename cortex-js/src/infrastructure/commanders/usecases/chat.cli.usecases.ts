import { ChatUsecases } from '@/usecases/chat/chat.usecases';
import { ChatCompletionRole } from '@/domain/models/message.interface';
import { exit, stdin, stdout } from 'node:process';
import * as readline from 'node:readline/promises';
import { ChatCompletionMessage } from '@/infrastructure/dtos/chat/chat-completion-message.dto';
import { CreateChatCompletionDto } from '@/infrastructure/dtos/chat/create-chat-completion.dto';
import { CortexUsecases } from '@/usecases/cortex/cortex.usecases';
import { Injectable } from '@nestjs/common';

@Injectable()
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

      const decoder = new TextDecoder('utf-8');
      this.chatUsecases.inference(chatDto, {}).then((response) => {
        response.on('error', (error: any) => {
          console.error(error);
          rl.prompt();
        });

        response.on('end', () => {
          console.log('\n');
          rl.prompt();
        });

        response.on('data', (chunk: any) => {
          let content = '';
          const text = decoder.decode(chunk);
          const lines = text.trim().split('\n');
          let cachedLines = '';
          for (const line of lines) {
            try {
              const toParse = cachedLines + line;
              if (!line.includes('data: [DONE]')) {
                const data = JSON.parse(toParse.replace('data: ', ''));
                content += data.choices[0]?.delta?.content ?? '';

                if (content.startsWith('assistant: ')) {
                  content = content.replace('assistant: ', '');
                }

                if (content.trim().length > 0) {
                  stdout.write(content);
                }
              }
            } catch {
              cachedLines = line;
            }
          }
        });
      });
    });
  }
}
