import { HttpService } from '@nestjs/axios';
import { OAIEngineExtension } from '../domain/abstracts/oai.abstract';
import { ConfigsUsecases } from '@/usecases/configs/configs.usecase';

/**
 * A class that implements the InferenceExtension interface from the @janhq/core package.
 * The class provides methods for initializing and stopping a model, and for making inference requests.
 * It also subscribes to events emitted by the @janhq/core package and handles new message requests.
 */
export default class MistralEngineExtension extends OAIEngineExtension {
  apiUrl = 'https://api.mistral.ai/v1/chat/completions';
  name = 'mistral';
  productName = 'Mistral Inference Engine';
  description = 'This extension enables Mistral chat completion API calls';
  version = '0.0.1';

  constructor(
    protected readonly httpService: HttpService,
    protected readonly configsUsecases: ConfigsUsecases,
  ) {
    super(httpService);
  }

  async onLoad() {
    const configs = (await this.configsUsecases.getGroupConfigs(
      this.name,
    )) as unknown as { apiKey: string };
    if (!configs?.apiKey)
      await this.configsUsecases.saveConfig('apiKey', '', this.name);
  }
}
