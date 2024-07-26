import { SubCommand, Option } from 'nest-commander';
import { exit } from 'node:process';
import { SetCommandContext } from '../decorators/CommandContext';
import { UpdateModelDto } from '@/infrastructure/dtos/models/update-model.dto';
import { ContextService } from '@/infrastructure/services/context/context.service';
import { BaseCommand } from '../base.command';
import { CortexUsecases } from '@/usecases/cortex/cortex.usecases';
import { CortexClient } from '../services/cortex.client';

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
@SetCommandContext()
export class ModelUpdateCommand extends BaseCommand {
  constructor(
    readonly contextService: ContextService,
    readonly cortexUseCases: CortexUsecases,
    private readonly cortex: CortexClient,
  ) {
    super(cortexUseCases);
  }

  async runCommand(
    passedParams: string[],
    option: UpdateOptions,
  ): Promise<void> {
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
    this.cortex.models.update(modelId, toUpdate as any);
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
