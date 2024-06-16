import { CommandRunner, SubCommand, Option } from 'nest-commander';
import { ModelsCliUsecases } from '@commanders/usecases/models.cli.usecases';
import { exit } from 'node:process';
import { UpdateModelDto } from '@/infrastructure/dtos/models/update-model.dto';

type UpdateOptions = {
  model?: string;
  options?: string[];
};

@SubCommand({
  name: 'update',
  description: 'Update configuration of a model.',
  arguments: '<model_id>',
  argsDescription: {
    model_id: 'Model ID to update configuration.',
  },
})
export class ModelUpdateCommand extends CommandRunner {
  constructor(private readonly modelsCliUsecases: ModelsCliUsecases) {
    super();
  }

  async run(passedParams: string[], option: UpdateOptions): Promise<void> {
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

    const toUpdate: UpdateModelDto = {};

    options.forEach((option) => {
      const [key, stringValue] = option.split('=');
      Object.assign(toUpdate, { key, stringValue });
    });
    this.modelsCliUsecases.updateModel(modelId, toUpdate);
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
