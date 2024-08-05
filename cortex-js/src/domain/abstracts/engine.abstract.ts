/* eslint-disable no-unused-vars, @typescript-eslint/no-unused-vars */
import stream from 'stream';
import { Model, ModelSettingParams } from '../../domain/models/model.interface';
import { Extension } from './extension.abstract';

export enum EngineStatus {
  READY = 'READY',
  MISSING_CONFIGURATION = 'MISSING_CONFIGURATION',
  NOT_INITIALIZED = 'NOT_INITIALIZED',
  NOT_SUPPORTED = 'NOT_SUPPORTED',
  ERROR = 'ERROR',
}

/**
 * This class should be extended by any class that represents an engine extension.
 * It provides methods for loading and unloading models, and for making inference requests.
 */
export abstract class EngineExtension extends Extension {
  abstract onLoad(): void;

  transformPayload?: Function;

  transformResponse?: Function;

  status: EngineStatus = EngineStatus.READY;

  /**
   * Makes an inference request to the engine.
   * @param dto
   * @param headers
   */
  abstract inference(
    dto: any,
    headers: Record<string, string>,
  ): Promise<stream.Readable | any>;

  /**
   * Checks if a model is running by the engine
   * This method should check run-time status of the model
   * Since the model can be corrupted during the run-time
   * This method should return false if the model is not running
   * @param modelId
   */
  async isModelRunning(modelId: string): Promise<boolean> {
    return true;
  }

  /**
   * Loads a model into the engine.
   * There are model settings such as `ngl` and `ctx_len` that can be passed to the engine.
   * Applicable for local engines only
   * @param model
   * @param settingParams
   */
  async loadModel(
    model: Model,
    settingParams?: ModelSettingParams,
  ): Promise<void> {}

  /**
   * Unloads a model from the engine.
   * @param modelId
   */
  async unloadModel(modelId: string, engine?: string): Promise<void> {}
}
