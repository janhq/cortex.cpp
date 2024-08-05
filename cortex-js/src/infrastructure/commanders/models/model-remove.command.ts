import { SubCommand } from 'nest-commander';
import { SetCommandContext } from '../decorators/CommandContext';
import { ContextService } from '@/infrastructure/services/context/context.service';
import { CortexUsecases } from '@/usecases/cortex/cortex.usecases';
import { BaseCommand } from '../base.command';
import { CortexClient } from '../services/cortex.client';

@SubCommand({
  name: 'remove',
  description: 'Remove a model by ID locally.',
  arguments: '<model_id>',
  argsDescription: {
    model_id: 'Model to remove',
  },
})
@SetCommandContext()
export class ModelRemoveCommand extends BaseCommand {
  constructor(
    readonly contextService: ContextService,
    readonly cortexUseCases: CortexUsecases,
    readonly cortex: CortexClient,
  ) {
    super(cortexUseCases);
  }

  async runCommand(passedParams: string[]): Promise<void> {
    await this.cortex.models.del(passedParams[0]).then(console.log);
  }
}
