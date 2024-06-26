/* eslint-disable no-unused-vars, @typescript-eslint/no-unused-vars */
import stream from 'stream';
import { Model, ModelSettingParams } from '../../domain/models/model.interface';
import { Extension } from './extension.abstract';

export abstract class EngineExtension extends Extension {
  abstract onLoad(): void;

  abstract inference(
    dto: any,
    headers: Record<string, string>,
  ): Promise<stream.Readable | any>;

  async loadModel(
    model: Model,
    settingParams?: ModelSettingParams,
  ): Promise<void> {}

  async unloadModel(modelId: string): Promise<void> {}
}
