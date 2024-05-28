import { ChatUsecases } from '@/usecases/chat/chat.usecases';
import {
  ChatCompletionRole,
  ContentType,
  MessageStatus,
} from '@/domain/models/message.interface';
import { exit, stdin, stdout } from 'node:process';
import * as readline from 'node:readline/promises';
import { ChatCompletionMessage } from '@/infrastructure/dtos/chat/chat-completion-message.dto';
import { CreateChatCompletionDto } from '@/infrastructure/dtos/chat/create-chat-completion.dto';
import { CortexUsecases } from '@/usecases/cortex/cortex.usecases';
import { Injectable } from '@nestjs/common';
import { ThreadsUsecases } from '@/usecases/threads/threads.usecases';
import { Thread } from '@/domain/models/thread.interface';
import { CreateThreadDto } from '@/infrastructure/dtos/threads/create-thread.dto';
import { AssistantsUsecases } from '@/usecases/assistants/assistants.usecases';
import { CreateThreadAssistantDto } from '@/infrastructure/dtos/threads/create-thread-assistant.dto';
import { CreateThreadModelInfoDto } from '@/infrastructure/dtos/threads/create-thread-model-info.dto';
import { ModelsUsecases } from '@/usecases/models/models.usecases';
import stream from 'stream';
import { CreateMessageDto } from '@/infrastructure/dtos/messages/create-message.dto';
import { MessagesUsecases } from '@/usecases/messages/messages.usecases';

@Injectable()
export class ChatCliUsecases {
  private exitClause = 'exit()';
  private userIndicator = '>> ';
  private exitMessage = 'Bye!';

  constructor(
    private readonly assistantUsecases: AssistantsUsecases,
    private readonly threadUsecases: ThreadsUsecases,
    private readonly chatUsecases: ChatUsecases,
    private readonly cortexUsecases: CortexUsecases,
    private readonly modelsUsecases: ModelsUsecases,
    private readonly messagesUsecases: MessagesUsecases,
  ) {}

  private async getOrCreateNewThread(
    modelId: string,
    threadId?: string,
  ): Promise<Thread> {
    if (threadId) {
      const thread = await this.threadUsecases.findOne(threadId);
      if (!thread) throw new Error(`Cannot find thread with id: ${threadId}`);
      return thread;
    }

    const model = await this.modelsUsecases.findOne(modelId);
    if (!model) throw new Error(`Cannot find model with id: ${modelId}`);

    const assistant = await this.assistantUsecases.findOne('jan');
    if (!assistant) throw new Error('No assistant available');

    const createThreadModel: CreateThreadModelInfoDto = {
      id: modelId,
      settings: model.settings,
      parameters: model.parameters,
    };

    const assistantDto: CreateThreadAssistantDto = {
      assistant_id: assistant.id,
      assistant_name: assistant.name,
      model: createThreadModel,
    };

    const createThreadDto: CreateThreadDto = {
      title: 'New Thread',
      assistants: [assistantDto],
    };

    return this.threadUsecases.create(createThreadDto);
  }

  async chat(modelId: string, threadId?: string): Promise<void> {
    console.log(`Inorder to exit, type '${this.exitClause}'.`);
    const thread = await this.getOrCreateNewThread(modelId, threadId);
    const messages: ChatCompletionMessage[] = (
      await this.messagesUsecases.getLastMessagesByThread(thread.id, 10)
    ).map((message) => ({
      content: message.content[0].text.value,
      role: message.role,
    }));

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

    const decoder = new TextDecoder('utf-8');

    rl.on('line', (userInput: string) => {
      if (userInput.trim() === this.exitClause) {
        rl.close();
        return;
      }

      messages.push({
        content: userInput,
        role: ChatCompletionRole.User,
      });

      const createMessageDto: CreateMessageDto = {
        thread_id: thread.id,
        role: ChatCompletionRole.User,
        content: [
          {
            type: ContentType.Text,
            text: {
              value: userInput,
              annotations: [],
            },
          },
        ],
        status: MessageStatus.Ready,
      };
      this.messagesUsecases.create(createMessageDto);

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

      this.chatUsecases
        .inference(chatDto, {})
        .then((response: stream.Readable) => {
          let assistantResponse: string = '';

          response.on('error', (error: any) => {
            console.error(error);
            rl.prompt();
          });

          response.on('end', () => {
            messages.push({
              content: assistantResponse,
              role: ChatCompletionRole.Assistant,
            });
            const createMessageDto: CreateMessageDto = {
              thread_id: thread.id,
              role: ChatCompletionRole.Assistant,
              content: [
                {
                  type: ContentType.Text,
                  text: {
                    value: assistantResponse,
                    annotations: [],
                  },
                },
              ],
              status: MessageStatus.Ready,
            };

            this.messagesUsecases.create(createMessageDto).then(() => {
              assistantResponse = '';
              console.log('\n');
              rl.prompt();
            });
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
                    assistantResponse += content;
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
