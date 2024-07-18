import { CommandRunner, SubCommand, Option } from 'nest-commander';
import { SetCommandContext } from '../decorators/CommandContext';
import { ContextService } from '@/infrastructure/services/context/context.service';
import { ConfigsUsecases } from '@/usecases/configs/configs.usecase';
import { CortexUsecases } from '@/usecases/cortex/cortex.usecases';
import { BaseCommand } from '../base.command';

interface ConfigsSetOption {
  key: string;
  value: string;
  group?: string;
}

@SubCommand({
  name: 'set',
  description: 'Set a cortex configuration',
})
@SetCommandContext()
export class ConfigsSetCommand extends BaseCommand {
  constructor(
    private readonly configsUsecases: ConfigsUsecases,
    readonly contextService: ContextService,
    readonly cortexUsecases: CortexUsecases,
  ) {
    super(cortexUsecases);
  }

  async runCommand(
    passedParams: string[],
    options: ConfigsSetOption,
  ): Promise<void> {
    return this.configsUsecases
      .saveConfig(options.key, options.value, options.group)
      .then(() => console.log('Set configuration successfully'));
  }

  @Option({
    flags: '-k, --key <key>',
    description: 'Configuration key',
  })
  parseKey(value: string) {
    return value;
  }

  @Option({
    flags: '-v, --value <value>',
    description: 'Configuration value',
  })
  parseValue(value: string) {
    return value;
  }

  @Option({
    flags: '-g, --group <group>',
    description: 'Configuration group',
  })
  parseGroup(value?: string) {
    return value;
  }
}
