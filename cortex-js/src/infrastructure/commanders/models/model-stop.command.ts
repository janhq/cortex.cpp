import { SubCommand } from 'nest-commander';
import { exit } from 'node:process';
import { ModelsCliUsecases } from '../usecases/models.cli.usecases';
import { SetCommandContext } from '../decorators/CommandContext';
import { ContextService } from '@/infrastructure/services/context/context.service';
import { BaseCommand } from '../base.command';
import { CortexUsecases } from '@/usecases/cortex/cortex.usecases';

@SubCommand({
  name: 'stop',
  description: 'Stop a model by ID.',
  arguments: '<model_id>',
  argsDescription: {
    model_id: 'Model ID to stop.',
  },
})
@SetCommandContext()
export class ModelStopCommand extends BaseCommand {
  constructor(
    private readonly modelsCliUsecases: ModelsCliUsecases,
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

    await this.modelsCliUsecases.stopModel(passedParams[0]).then(console.log);
  }
}
