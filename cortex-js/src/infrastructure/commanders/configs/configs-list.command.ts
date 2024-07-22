import { SubCommand } from 'nest-commander';
import { SetCommandContext } from '../decorators/CommandContext';
import { ContextService } from '@/infrastructure/services/context/context.service';
import { ConfigsUsecases } from '@/usecases/configs/configs.usecase';
import { BaseCommand } from '../base.command';
import { CortexUsecases } from '@/usecases/cortex/cortex.usecases';

@SubCommand({
  name: 'list',
  description: 'Get all cortex configurations',
})
@SetCommandContext()
export class ConfigsListCommand extends BaseCommand {
  constructor(
    private readonly configsUsecases: ConfigsUsecases,
    readonly contextService: ContextService,
    readonly cortexUsecases: CortexUsecases,
  ) {
    super(cortexUsecases);
  }

  async runCommand(): Promise<void> {
    return this.configsUsecases.getConfigs().then(console.table);
  }
}
