import { SubCommand } from 'nest-commander';
import { SetCommandContext } from '../decorators/CommandContext';
import { ContextService } from '@/infrastructure/services/context/context.service';
import { BaseCommand } from '../base.command';
import { CortexUsecases } from '@/usecases/cortex/cortex.usecases';
import { EnginesUsecases } from '@/usecases/engines/engines.usecase';

@SubCommand({
  name: '<name> set <config> <value>',
  description: 'Update an engine configurations',
  argsDescription: {
    name: 'Engine name to update',
  },
})
@SetCommandContext()
export class EnginesSetCommand extends BaseCommand {
  constructor(
    readonly contextService: ContextService,
    readonly cortexUsecases: CortexUsecases,
    readonly engineUsecases: EnginesUsecases,
  ) {
    super(cortexUsecases);
  }

  async runCommand(passedParams: string[]): Promise<void> {
    const engineName = passedParams[0];
    const config = passedParams[1];
    const value = passedParams[2];
    return this.engineUsecases
      .updateConfigs(config, value, engineName)
      .then(() => console.log('Update engine successfully'))
      .catch((error) => console.error(error.message ?? error));
  }
}
