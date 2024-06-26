import { HttpService } from '@nestjs/axios';
import { OAIEngineExtension } from '../domain/abstracts/oai.abstract';
import { ConfigsUsecases } from '@/usecases/configs/configs.usecase';

/**
 * A class that implements the InferenceExtension interface from the @janhq/core package.
 * The class provides methods for initializing and stopping a model, and for making inference requests.
 * It also subscribes to events emitted by the @janhq/core package and handles new message requests.
 */
export default class OpenAIEngineExtension extends OAIEngineExtension {
  provider: string = 'openai';
  apiUrl = 'https://api.openai.com/v1/chat/completions';
  name = 'OpenAI Inference Engine';
  description? = 'This extension enables OpenAI chat completion API calls';

  constructor(
    protected readonly httpService: HttpService,
    protected readonly configsUsecases: ConfigsUsecases,
  ) {
    super(httpService);
  }

  async onLoad() {
    const configs = (await this.configsUsecases.getGroupConfigs(
      this.provider,
    )) as unknown as { apiKey: string };
    if (!configs?.apiKey)
      await this.configsUsecases.saveConfig('apiKey', '', this.provider);
  }
}
