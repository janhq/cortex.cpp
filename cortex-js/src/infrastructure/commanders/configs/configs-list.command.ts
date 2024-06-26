import { CommandRunner, SubCommand } from 'nest-commander';
import { SetCommandContext } from '../decorators/CommandContext';
import { ContextService } from '@/infrastructure/services/context/context.service';
import { ConfigsUsecases } from '@/usecases/configs/configs.usecase';

@SubCommand({
  name: 'list',
  description: 'Get all cortex configurations',
})
@SetCommandContext()
export class ConfigsListCommand extends CommandRunner {
  constructor(
    private readonly configsUsecases: ConfigsUsecases,
    readonly contextService: ContextService,
  ) {
    super();
  }

  async run(): Promise<void> {
    return this.configsUsecases.getConfigs().then(console.table);
  }
}
