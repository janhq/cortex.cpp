import stream from 'stream';
import { HttpService } from '@nestjs/axios';
import { OAIEngineExtension } from '../domain/abstracts/oai.abstract';
import { ConfigsUsecases } from '@/usecases/configs/configs.usecase';
import { EventEmitter2 } from '@nestjs/event-emitter';
import _ from 'lodash';
import { EngineStatus } from '@/domain/abstracts/engine.abstract';

enum RoleType {
    user = 'USER',
    chatbot = 'CHATBOT',
    system = 'SYSTEM',
  }
  
  type CoherePayloadType = {
    chat_history?: Array<{ role: RoleType; message: string }>
    message?: string
    preamble?: string
  }

/**
 * A class that implements the InferenceExtension interface from the @janhq/core package.
 * The class provides methods for initializing and stopping a model, and for making inference requests.
 * It also subscribes to events emitted by the @janhq/core package and handles new message requests.
 */
export default class CoHereEngineExtension extends OAIEngineExtension {
  apiUrl = 'https://api.cohere.ai/v1/chat';
  name = 'cohere';
  productName = 'Cohere Inference Engine';
  description = 'This extension enables Cohere chat completion API calls';
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

  transformPayload = (payload: any): CoherePayloadType => {
    console.log('payload', payload)
    if (payload.messages.length === 0) {
      return {}
    }

    const { messages, ...params } = payload
    const convertedData: CoherePayloadType = {
      ...params,
      chat_history: [],
      message: '',
    }
    (messages as any).forEach((item: any, index: number) => {
      // Assign the message of the last item to the `message` property
      if (index === messages.length - 1) {
        convertedData.message = item.content as string
        return
      }
      if (item.role === 'user') {
        convertedData.chat_history!!.push({
          role: 'USER' as RoleType,
          message: item.content as string,
        })
      } else if (item.role === 'assistant') {
        convertedData.chat_history!!.push({
          role: 'CHATBOT' as RoleType,
          message: item.content as string,
        })
      } else if (item.role === 'system') {
        convertedData.preamble = item.content as string
      }
    })
    return convertedData
  }

  transformResponse = (data: any) => {
    const text =  typeof data === 'object' ? data.text : JSON.parse(data).text ?? ''
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
}
