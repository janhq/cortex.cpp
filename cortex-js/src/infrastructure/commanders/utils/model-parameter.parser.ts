import {
  Model,
  ModelRuntimeParams,
  ModelSettingParams,
} from '@/domain/models/model.interface';

// Make this class injectable
export class ModelParameterParser {
  private modelSettingParamTypes: { [key: string]: string } = {
    prompt_template: 'string',
    ctx_len: 'number',
    ngl: 'number',
    n_parallel: 'number',
    cpu_threads: 'number',
    llama_model_path: 'string',
    mmproj: 'string',
    cont_batching: 'boolean',
    pre_prompt: 'string',
  };

  private modelRuntimeParamTypes: { [key: string]: string } = {
    temperature: 'number',
    top_k: 'number',
    top_p: 'number',
    stream: 'boolean',
    max_tokens: 'number',
    stop: 'string[]',
    frequency_penalty: 'number',
    presence_penalty: 'number',
  };

  /**
   * Parse the model inference parameters from origin Model
   * @param model
   * @returns Partial<Model>
   */
  parseModelInferenceParams(model: Partial<Model>): Partial<Model> {
    const inferenceParams: Partial<Model> & ModelRuntimeParams =
      structuredClone(model);
    return Object.keys(inferenceParams).reduce((acc, key) => {
      if (!this.isModelRuntimeParam(key)) {
        delete acc[key as keyof typeof acc];
      }

      return acc;
    }, inferenceParams);
  }
  /**
   * Parse the model engine settings from origin Model
   * @param model
   * @returns Partial<Model>
   */
  parseModelEngineSettings(model: Partial<Model>): Partial<Model> {
    const engineSettings: Partial<Model> & ModelSettingParams =
      structuredClone(model);
    return Object.keys(engineSettings).reduce((acc, key) => {
      if (!this.isModelSettingParam(key)) {
        delete acc[key as keyof typeof acc];
      }

      return acc;
    }, engineSettings);
  }

  private isModelSettingParam(key: string): boolean {
    return key in this.modelSettingParamTypes;
  }

  private isModelRuntimeParam(key: string): boolean {
    return key in this.modelRuntimeParamTypes;
  }
}
