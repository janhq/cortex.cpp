import { OAIEngineExtension } from '@janhq/core';

/**
 * A class that implements the InferenceExtension interface from the @janhq/core package.
 * The class provides methods for initializing and stopping a model, and for making inference requests.
 * It also subscribes to events emitted by the @janhq/core package and handles new message requests.
 */
export default class MistralEngineExtension extends OAIEngineExtension {
  provider: string = 'mistral';
  apiUrl = 'https://api.mistral.ai/v1/chat/completions';
}
