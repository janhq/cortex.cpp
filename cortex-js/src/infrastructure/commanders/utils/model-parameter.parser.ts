// Make this class injectable
export class ModelParameterParser {
  private modelSettingParamTypes: { [key: string]: string } = {
    ctx_len: 'number',
    ngl: 'number',
    embedding: 'boolean',
    n_parallel: 'number',
    cpu_threads: 'number',
    prompt_template: 'string',
    system_prompt: 'string',
    ai_prompt: 'string',
    user_prompt: 'string',
    llama_model_path: 'string',
    mmproj: 'string',
    cont_batching: 'boolean',
    vision_model: 'boolean',
    text_model: 'boolean',
  };

  private modelRuntimeParamTypes: { [key: string]: string } = {
    temperature: 'number',
    token_limit: 'number',
    top_k: 'number',
    top_p: 'number',
    stream: 'boolean',
    max_tokens: 'number',
    stop: 'string[]',
    frequency_penalty: 'number',
    presence_penalty: 'number',
  };

  isModelSettingParam(key: string): boolean {
    return key in this.modelSettingParamTypes;
  }

  isModelRuntimeParam(key: string): boolean {
    return key in this.modelRuntimeParamTypes;
  }

  parse(key: string, value: string): boolean | number | string | string[] {
    if (this.isModelSettingParam(key)) {
      return this.parseModelSettingParams(key, value);
    }

    if (this.isModelRuntimeParam(key)) {
      return this.parseModelRuntimeParams(key, value);
    }

    throw new Error(`Invalid setting key: ${key}`);
  }

  private parseModelSettingParams(
    key: string,
    value: string,
  ): boolean | number | string | string[] {
    const settingType = this.modelSettingParamTypes[key];
    if (!settingType) {
      throw new Error(`Invalid setting key: ${key}`);
    }

    switch (settingType) {
      case 'string':
        return value;

      case 'number':
        return this.toNumber(value);

      case 'string[]':
        return this.toStringArray(value);

      case 'boolean':
        return this.toBoolean(value);

      default:
        throw new Error('Invalid setting type');
    }
  }

  private parseModelRuntimeParams(
    key: string,
    value: string,
  ): boolean | number | string | string[] {
    const settingType = this.modelRuntimeParamTypes[key];
    if (!settingType) {
      throw new Error(`Invalid setting key: ${key}`);
    }

    switch (settingType) {
      case 'string':
        return value;

      case 'number':
        return this.toNumber(value);

      case 'string[]':
        return this.toStringArray(value);

      case 'boolean':
        return this.toBoolean(value);

      default:
        throw new Error('Invalid setting type');
    }
  }

  private toNumber(str: string): number {
    const num = parseFloat(str.trim());
    if (isNaN(num)) {
      throw new Error(`Invalid number value: ${str}`);
    }
    return num;
  }

  private toStringArray(str: string, delimiter: string = ','): string[] {
    return str.split(delimiter).map((s) => s.trim());
  }

  private toBoolean(str: string): boolean {
    const normalizedStr = str.trim().toLowerCase();
    switch (normalizedStr) {
      case '1':
      case 'true':
        return true;

      case '0':
      case 'false':
        return false;

      default:
        throw new Error(`Invalid boolean value: ${str}`);
    }
  }
}
