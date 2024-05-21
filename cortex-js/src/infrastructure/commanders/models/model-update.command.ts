import { CommandRunner, SubCommand, Option } from 'nest-commander';
import { ModelsCliUsecases } from '../usecases/models.cli.usecases';
import { exit } from 'node:process';
import { ModelParameterParser } from '../utils/model-parameter.parser';
import {
  ModelRuntimeParams,
  ModelSettingParams,
} from '@/domain/models/model.interface';

type UpdateOptions = {
  model?: string;
  options?: string[];
};

@SubCommand({ name: 'update', description: 'Update configuration of a model.' })
export class ModelUpdateCommand extends CommandRunner {
  constructor(private readonly modelsCliUsecases: ModelsCliUsecases) {
    super();
  }

  async run(_input: string[], option: UpdateOptions): Promise<void> {
    const modelId = option.model;
    if (!modelId) {
      console.error('Model Id is required');
      exit(1);
    }

    const options = option.options;
    if (!options || options.length === 0) {
      console.log('Nothing to update');
      exit(0);
    }

    const parser = new ModelParameterParser();
    const settingParams: ModelSettingParams = {};
    const runtimeParams: ModelRuntimeParams = {};

    options.forEach((option) => {
      const [key, stringValue] = option.split('=');
      if (parser.isModelSettingParam(key)) {
        const value = parser.parse(key, stringValue);
        // @ts-expect-error did the check so it's safe
        settingParams[key] = value;
      } else if (parser.isModelRuntimeParam(key)) {
        const value = parser.parse(key, stringValue);
        // @ts-expect-error did the check so it's safe
        runtimeParams[key] = value;
      }
    });

    if (Object.keys(settingParams).length > 0) {
      const updatedSettingParams =
        await this.modelsCliUsecases.updateModelSettingParams(
          modelId,
          settingParams,
        );
      console.log(
        'Updated setting params! New setting params:',
        updatedSettingParams,
      );
    }

    if (Object.keys(runtimeParams).length > 0) {
      await this.modelsCliUsecases.updateModelRuntimeParams(
        modelId,
        runtimeParams,
      );
      console.log('Updated runtime params! New runtime params:', runtimeParams);
    }
  }

  @Option({
    flags: '-m, --model <model_id>',
    required: true,
    description: 'Model Id to update',
  })
  parseModelId(value: string) {
    return value;
  }

  @Option({
    flags: '-c, --options <options...>',
    description:
      'Specify the options to update the model. Syntax: -c option1=value1 option2=value2. For example: cortex models update -c max_tokens=100 temperature=0.5',
  })
  parseOptions(option: string, optionsAccumulator: string[] = []): string[] {
    optionsAccumulator.push(option);
    return optionsAccumulator;
  }
}
