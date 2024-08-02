import { SubCommand } from 'nest-commander';
import { SetCommandContext } from '../decorators/CommandContext';
import { BaseCommand } from '../base.command';
import { CortexUsecases } from '@/usecases/cortex/cortex.usecases';
import { FileManagerService } from '@/infrastructure/services/file-manager/file-manager.service';

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
    readonly cortexUsecases: CortexUsecases,
    readonly fileService: FileManagerService,
  ) {
    super(cortexUsecases, fileService);
  }

  async runCommand(passedParams: string[]): Promise<void> {
    const engineName = passedParams[0];
    const config = passedParams[1];
    const value = passedParams[2];
    return this.cortex.engines
      .update(engineName, { config, value })
      .then(() => console.log('Update engine successfully'))
      .catch((error) => console.error(error.message ?? error));
  }
}
