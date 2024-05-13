import { OAIEngineExtension } from './../../domain/abstracts/oai.abstract';

/**
 * A class that implements the InferenceExtension interface from the @janhq/core package.
 * The class provides methods for initializing and stopping a model, and for making inference requests.
 * It also subscribes to events emitted by the @janhq/core package and handles new message requests.
 */
export default class OpenAIEngineExtension extends OAIEngineExtension {
  provider: string = 'openai';
  apiUrl = 'https://api.openai.com/v1/chat/completions';
}
