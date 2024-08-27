import stream from 'stream';
import { HttpService } from '@nestjs/axios';
import { OAIEngineExtension } from '../domain/abstracts/oai.abstract';
import { ConfigsUsecases } from '@/usecases/configs/configs.usecase';
import { EventEmitter2 } from '@nestjs/event-emitter';
import { pick } from 'lodash';
import { EngineStatus } from '@/domain/abstracts/engine.abstract';

/**
 * A class that implements the InferenceExtension interface from the @janhq/core package.
 * The class provides methods for initializing and stopping a model, and for making inference requests.
 * It also subscribes to events emitted by the @janhq/core package and handles new message requests.
 */
export default class AnthropicEngineExtension extends OAIEngineExtension {
  apiUrl = 'https://api.anthropic.com/v1/messages';
  name = 'anthropic';
  productName = 'Anthropic Inference Engine';
  description = 'This extension enables Anthropic chat completion API calls';
  version = '0.0.1';
  apiKey?: string;

  constructor(
    protected readonly httpService: HttpService,
    protected readonly configsUsecases: ConfigsUsecases,
    protected readonly eventEmmitter: EventEmitter2,
  ) {
    super(httpService);

    eventEmmitter.on('config.updated', async (data) => {
      if (data.engine === this.name) {
        this.apiKey = data.value;
        this.status =
          (this.apiKey?.length ?? 0) > 0
            ? EngineStatus.READY
            : EngineStatus.MISSING_CONFIGURATION;
      }
    });
  }

  async onLoad() {
    const configs = (await this.configsUsecases.getGroupConfigs(
      this.name,
    )) as unknown as { apiKey: string };
    this.apiKey = configs?.apiKey;
    this.status =
      (this.apiKey?.length ?? 0) > 0
        ? EngineStatus.READY
        : EngineStatus.MISSING_CONFIGURATION;
  }

  override async inference(
    dto: any,
    headers: Record<string, string>,
  ): Promise<stream.Readable | any> {
    headers['x-api-key'] = this.apiKey as string;
    headers['Content-Type'] = 'application/json';
    headers['anthropic-version'] = '2023-06-01';
    return super.inference(dto, headers);
  }

  transformPayload = (data: any): any => {
    const system = data.messages.find((m: any) => m.role === 'system');
    const messages = data.messages.filter((m: any) => m.role !== 'system');
    return {
      system: system?.content ?? '',
      messages,
      ...pick(data, ['model', 'stream', 'max_tokens']),
    };
  };

  transformResponse = (data: any) => {
    // handling stream response
    if (typeof data === 'string' && data.trim().length === 0) {
      return '';
    }
    if (typeof data === 'string' && data.startsWith('event: ')) {
      return '';
    }
    if (typeof data === 'string' && data.startsWith('data: ')) {
      data = data.replace('data: ', '');
      const parsedData = JSON.parse(data);
      if (parsedData.type !== 'content_block_delta') {
        return '';
      }
      const text = parsedData.delta?.text;
      //convert to have this format data.choices[0]?.delta?.content
      return JSON.stringify({
        choices: [
          {
            delta: {
              content: text,
            },
          },
        ],
      });
    }
    // non-stream response
    if (data.content && data.content.length > 0 && data.content[0].text) {
      return {
        choices: [
          {
            message: {
              content: data.content[0].text,
            },
          },
        ],
      };
    }

    console.error('Invalid response format:', data);
    return '';
  };
}
