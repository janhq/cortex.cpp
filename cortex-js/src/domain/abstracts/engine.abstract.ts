import { Extension } from './extension.abstract';

export abstract class EngineExtension extends Extension {
  abstract provider: string;
  abstract inference(completion: any, req: any, res: any): void;
  abstract loadModel(loadModel: any): Promise<void>;
}
