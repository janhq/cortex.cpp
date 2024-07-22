import { SubCommand } from 'nest-commander';
import { CortexUsecases } from '@/usecases/cortex/cortex.usecases';
import { SetCommandContext } from './decorators/CommandContext';
import { ContextService } from '../services/context/context.service';
import { BaseCommand } from './base.command';

@SubCommand({
  name: 'kill',
  description: 'Kill running cortex processes',
})
@SetCommandContext()
export class KillCommand extends BaseCommand {
  constructor(
    private readonly cortexUsecases: CortexUsecases,
    readonly contextService: ContextService,
  ) {
    super(cortexUsecases);
  }
  async runCommand(): Promise<void> {
    return this.cortexUsecases
      .stopCortex()
      .then(this.cortexUsecases.stopServe)
      .then(() => console.log('Cortex processes stopped successfully!'));
  }
}
