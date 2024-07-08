import { exit, stdin, stdout } from 'node:process';
import * as readline from 'node:readline/promises';
import { ChatCompletionMessage } from '@/infrastructure/dtos/chat/chat-completion-message.dto';
import { CreateChatCompletionDto } from '@/infrastructure/dtos/chat/create-chat-completion.dto';
import { Injectable } from '@nestjs/common';
import { ThreadsUsecases } from '@/usecases/threads/threads.usecases';
import { Thread } from '@/domain/models/thread.interface';
import { CreateThreadDto } from '@/infrastructure/dtos/threads/create-thread.dto';
import { AssistantsUsecases } from '@/usecases/assistants/assistants.usecases';
import { CreateThreadAssistantDto } from '@/infrastructure/dtos/threads/create-thread-assistant.dto';
import { ModelsUsecases } from '@/usecases/models/models.usecases';
import stream from 'stream';
import { CreateMessageDto } from '@/infrastructure/dtos/messages/create-message.dto';
import { MessagesUsecases } from '@/usecases/messages/messages.usecases';
import { ModelParameterParser } from '@/utils/model-parameter.parser';
import { ChatUsecases } from '@/usecases/chat/chat.usecases';
import { TextContentBlock } from '@/domain/models/message.interface';

@Injectable()
export class ChatCliUsecases {
  private exitClause = 'exit()';
  private userIndicator = '>> ';
  private exitMessage = 'Bye!';

  constructor(
    private readonly assistantUsecases: AssistantsUsecases,
    private readonly threadUsecases: ThreadsUsecases,
    private readonly chatUsecases: ChatUsecases,
    private readonly modelsUsecases: ModelsUsecases,
    private readonly messagesUsecases: MessagesUsecases,
  ) {}

  async chat(
    modelId: string,
    threadId?: string,
    message?: string,
    attach: boolean = true,
    stopModel: boolean = true,
  ): Promise<void> {
    if (attach) console.log(`Inorder to exit, type '${this.exitClause}'.`);
    const thread = await this.getOrCreateNewThread(modelId, threadId);
    const messages: ChatCompletionMessage[] = (
      await this.messagesUsecases.getLastMessagesByThread(thread.id, 10)
    ).map((message) => ({
      content: (message.content[0] as TextContentBlock).text.value,
      role: message.role,
    }));

    const rl = readline.createInterface({
      input: stdin,
      output: stdout,
      prompt: this.userIndicator,
    });
    if (message) sendCompletionMessage.bind(this)(message);
    if (attach) rl.prompt();

    rl.on('close', async () => {
      if (stopModel) await this.modelsUsecases.stopModel(modelId);
      if (attach) console.log(this.exitMessage);
      exit(0);
    });

    rl.on('line', sendCompletionMessage.bind(this));

    async function sendCompletionMessage(userInput: string) {
      if (!userInput || userInput.trim() === '') return;

      if (userInput.trim() === this.exitClause) {
        rl.close();
        return;
      }

      const model = await this.modelsUsecases.findOne(modelId);

      messages.push({
        content: userInput,
        role: 'user',
      });

      const createMessageDto: CreateMessageDto = {
        thread_id: thread.id,
        role: 'user',
        content: [
          {
            type: 'text',
            text: {
              value: userInput,
              annotations: [],
            },
          },
        ],
        status: 'completed',
      };
      this.messagesUsecases.create(createMessageDto);

      const parser = new ModelParameterParser();
      const chatDto: CreateChatCompletionDto = {
        // Default results params
        messages,
        model: modelId,
        stream: true,
        max_tokens: 4098,
        temperature: 0.7,
        // Override with model settings
        ...parser.parseModelInferenceParams(model),
      };

      const decoder = new TextDecoder('utf-8');

      this.chatUsecases
        .inference(chatDto, {})

        .then((response: stream.Readable) => {
          // None streaming - json object response
          if (!chatDto.stream) {
            const objectData = response as any;
            const assistantResponse =
              objectData.choices[0]?.message?.content ?? '';

            stdout.write(assistantResponse);
            messages.push({
              content: assistantResponse,
              role: 'assistant',
            });

            const createMessageDto: CreateMessageDto = {
              thread_id: thread.id,
              role: 'assistant',
              content: [
                {
                  type: 'text',
                  text: {
                    value: assistantResponse,
                    annotations: [],
                  },
                },
              ],
              status: 'completed',
            };

            this.messagesUsecases.create(createMessageDto).then(() => {
              console.log('\n');
              if (attach) rl.prompt();
              else rl.close();
            });
            return;
          }
          // Streaming
          let assistantResponse: string = '';

          response.on('error', (error: any) => {
            console.error(error.message ?? error);
            if (attach) rl.prompt();
            else rl.close();
          });

          response.on('end', () => {
            messages.push({
              content: assistantResponse,
              role: 'assistant',
            });
            const createMessageDto: CreateMessageDto = {
              thread_id: thread.id,
              role: 'assistant',
              content: [
                {
                  type: 'text',
                  text: {
                    value: assistantResponse,
                    annotations: [],
                  },
                },
              ],
              status: 'completed',
            };

            this.messagesUsecases.create(createMessageDto).then(() => {
              assistantResponse = '';
              console.log('\n');
              if (attach) rl.prompt();
              else rl.close();
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
                  content = data.choices[0]?.delta?.content ?? '';

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
        })
        .catch((e: any) => {
          stdout.write(
            `Something went wrong! Please check model status.\n${e.message}\n`,
          );
          if (attach) rl.prompt();
          else rl.close();
        });
    }
  }

  /**
   * Creates an embedding vector representing the input text.
   * @param model Embedding model ID.
   * @param input Input text to embed, encoded as a string or array of tokens. To embed multiple inputs in a single request, pass an array of strings or array of token arrays.
   * @param encoding_format Encoding format for the embeddings. Supported formats are 'float' and 'int'.
   * @param dimensions The number of dimensions the resulting output embeddings should have. Only supported in some models.
   * @param host Cortex CPP host.
   * @param port Cortex CPP port.
   * @returns Embedding vector.
   */
  embeddings(
    model: string,
    input: string | string[],
    encoding_format: string = 'float',
    dimensions?: number,
  ) {
    return this.chatUsecases.embeddings({
      model,
      input,
      encoding_format,
      dimensions,
    });
  }

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

    const assistantDto: CreateThreadAssistantDto = {
      avatar: '',
      id: 'jan',
      object: 'assistant',
      created_at: Date.now(),
      name: 'Jan',
      description: 'A default assistant that can use all downloaded models',
      model: modelId,
      instructions: '',
      tools: [],
      metadata: {},
    };

    const createThreadDto: CreateThreadDto = {
      assistants: [assistantDto],
    };

    return this.threadUsecases.create(createThreadDto);
  }
}
