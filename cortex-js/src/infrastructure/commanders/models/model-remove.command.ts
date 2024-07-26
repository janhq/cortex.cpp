import { SubCommand } from 'nest-commander';
import { exit } from 'node:process';
import { SetCommandContext } from '../decorators/CommandContext';
import { ContextService } from '@/infrastructure/services/context/context.service';
import { CortexUsecases } from '@/usecases/cortex/cortex.usecases';
import { BaseCommand } from '../base.command';
import { BaseSubCommand } from '../base.subcommand';

@SubCommand({
  name: 'remove',
  description: 'Remove a model by ID locally.',
  arguments: '<model_id>',
  argsDescription: {
    model_id: 'Model to remove',
  },
})
@SetCommandContext()
export class ModelRemoveCommand extends BaseSubCommand {
  constructor(
    readonly contextService: ContextService,
    readonly cortexUseCases: CortexUsecases,
  ) {
    super(cortexUseCases);
  }

  async runCommand(passedParams: string[]): Promise<void> {
    await this.cortex.models.del(passedParams[0]).then(console.log);
  }
}
