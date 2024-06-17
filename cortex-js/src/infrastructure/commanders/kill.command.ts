import { CommandRunner, SubCommand } from 'nest-commander';
import { CortexUsecases } from '@/usecases/cortex/cortex.usecases';
import { SetCommandContext } from './decorators/CommandContext';
import { ContextService } from '@/util/context.service';

@SubCommand({
  name: 'kill',
  description: 'Kill running cortex processes',
})
@SetCommandContext()
export class KillCommand extends CommandRunner {
  constructor(
    private readonly usecases: CortexUsecases,
    readonly contextService: ContextService,
  ) {
    super();
  }
  async run(): Promise<void> {
    this.usecases.stopCortex().then(console.log);
  }
}
