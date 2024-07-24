import { exit, stdin, stdout } from 'node:process';
import * as readline from 'node:readline/promises';
import { ChatCompletionMessage } from '@/infrastructure/dtos/chat/chat-completion-message.dto';
import { CreateThreadDto } from '@/infrastructure/dtos/threads/create-thread.dto';
import { CreateThreadAssistantDto } from '@/infrastructure/dtos/threads/create-thread-assistant.dto';
import { CreateMessageDto } from '@/infrastructure/dtos/messages/create-message.dto';
import { ModelParameterParser } from '@/utils/model-parameter.parser';
import { TextContentBlock } from '@/domain/models/message.interface';
import { Cortex } from 'cortexso-node';
import { ChatCompletionCreateParamsStreaming } from 'cortexso-node/dist/resources';

export class ChatClient {
  private exitClause = 'exit()';
  private userIndicator = '>> ';
  private exitMessage = 'Bye!';

  constructor(readonly cortex: Cortex) {}

  async chat(
    modelId: string,
    threadId?: string,
    message?: string,
  ): Promise<void> {
    console.log(`In order to exit, type '${this.exitClause}'.`);
    const thread = await this.getOrCreateNewThread(modelId, threadId);
    const messages: ChatCompletionMessage[] = (
      await this.cortex.beta.threads.messages.list(thread.id, {
        limit: 10,
        order: 'desc',
      })
    ).data.map((message) => ({
      content: (message.content[0] as TextContentBlock).text.value,
      role: message.role,
    }));

    const rl = readline.createInterface({
      input: stdin,
      output: stdout,
      prompt: this.userIndicator,
    });
    if (message)
      this.sendCompletionMessage(message, messages, modelId, thread.id, rl);
    rl.prompt();

    rl.on('close', async () => {
      console.log(this.exitMessage);
      exit(0);
    });

    rl.on('line', (input) =>
      this.sendCompletionMessage(input, messages, modelId, thread.id, rl),
    );
  }

  async sendCompletionMessage(
    userInput: string,
    messages: Cortex.ChatCompletionMessageParam[],
    modelId: string,
    threadId: string,
    rl: readline.Interface,
  ) {
    if (!userInput || userInput.trim() === '') return;

    if (userInput.trim() === this.exitClause) {
      rl.close();
      return;
    }

    const model = await this.cortex.models.retrieve(modelId);

    messages.push({
      content: userInput,
      role: 'user',
    });

    const createMessageDto: CreateMessageDto = {
      thread_id: threadId,
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
    this.cortex.beta.threads.messages.create(threadId, createMessageDto);

    const parser = new ModelParameterParser();
    const chatDto: ChatCompletionCreateParamsStreaming = {
      // Default results params
      messages,
      model: modelId,
      max_tokens: 4098,
      temperature: 0.7,
      // Override with model inference params
      ...parser.parseModelInferenceParams(model),
      stream: true,
    };

    try {
      const stream = await this.cortex.chat.completions.create({
        ...chatDto,
      });

      let assistantResponse = '';
      for await (const part of stream) {
        const output = part.choices[0]?.delta?.content || '';
        assistantResponse += output;
        stdout.write(output);
      }

      messages.push({
        content: assistantResponse,
        role: 'assistant',
      });

      this.cortex.beta.threads.messages
        .create(threadId, {
          role: 'assistant',
          content: [
            {
              type: 'text',
              text: assistantResponse,
            },
          ],
        })
        .then(() => {
          assistantResponse = '';
          console.log('\n');
          rl.prompt();
        });
    } catch (e) {
      stdout.write(
        `Something went wrong! Please check model status.\n${e.message}\n`,
      );
      rl.prompt();
    }
  }

  // Get or create a new thread
  private async getOrCreateNewThread(
    modelId: string,
    threadId?: string,
  ): Promise<Cortex.Beta.Thread> {
    if (threadId) {
      const thread = await this.cortex.beta.threads.retrieve(threadId);
      if (!thread) throw new Error(`Cannot find thread with id: ${threadId}`);
      return thread;
    }

    const model = await this.cortex.models.retrieve(modelId);
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

    return this.cortex.beta.threads.create(createThreadDto as any);
  }
}
