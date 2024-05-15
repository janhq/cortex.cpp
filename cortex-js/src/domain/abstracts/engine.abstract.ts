import { Model } from '../models/model.interface';
import { Extension } from './extension.abstract';

export abstract class EngineExtension extends Extension {
  abstract provider: string;

  abstract inference(completion: any, req: any, stream: any, res?: any): void;

  async loadModel(model: Model): Promise<void> {}

  async unloadModel(modelId: string): Promise<void> {}
}
