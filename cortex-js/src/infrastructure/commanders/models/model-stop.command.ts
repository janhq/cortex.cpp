import { SubCommand } from 'nest-commander';
import { SetCommandContext } from '../decorators/CommandContext';
import { ContextService } from '@/infrastructure/services/context/context.service';
import { BaseCommand } from '../base.command';
import { CortexUsecases } from '@/usecases/cortex/cortex.usecases';
import ora from 'ora';
import { CortexClient } from '../services/cortex.client';

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
    readonly contextService: ContextService,
    readonly cortexUseCases: CortexUsecases,
    readonly cortex: CortexClient,
  ) {
    super(cortexUseCases);
  }

  async runCommand(passedParams: string[]): Promise<void> {
    const loadingSpinner = ora('Unloading model...').start();
    await this.cortex.models
      .stop(passedParams[0])
      .then(() => loadingSpinner.succeed('Model unloaded'))
      .catch((e) =>
        loadingSpinner.fail(`Failed to unload model: ${e.message ?? e}`),
      );
  }
}
