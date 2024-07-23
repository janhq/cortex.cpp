import { SubCommand } from 'nest-commander';
import { exit } from 'node:process';
import { SetCommandContext } from '../decorators/CommandContext';
import { ContextService } from '@/infrastructure/services/context/context.service';
import { BaseCommand } from '../base.command';
import { CortexUsecases } from '@/usecases/cortex/cortex.usecases';

@SubCommand({
  name: 'get',
  description: 'Get a model by ID.',
  arguments: '<model_id>',
  argsDescription: {
    model_id: 'Model ID to get information about.',
  },
})
@SetCommandContext()
export class ModelGetCommand extends BaseCommand {
  constructor(
    readonly contextService: ContextService,
    readonly cortexUseCases: CortexUsecases,
  ) {
    super(cortexUseCases);
  }

  async runCommand(passedParams: string[]): Promise<void> {
    if (passedParams.length === 0) {
      console.error('Model ID is required');
      exit(1);
    }

    const model = await this.cortex.models.retrieve(passedParams[0]);
    if (!model) console.error('Model not found');
    else console.log(model);
  }
}
