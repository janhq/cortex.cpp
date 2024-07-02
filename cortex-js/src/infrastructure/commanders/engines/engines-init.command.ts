import { CommandRunner, SubCommand } from 'nest-commander';
import { SetCommandContext } from '../decorators/CommandContext';
import { ContextService } from '@/infrastructure/services/context/context.service';
import { InitCliUsecases } from '../usecases/init.cli.usecases';
import { Engines } from '../types/engine.interface';

@SubCommand({
  name: 'init',
  description: 'Setup engine',
  arguments: '<name>',
  argsDescription: {
    name: 'Engine name to setup',
  },
})
@SetCommandContext()
export class EnginesInitCommand extends CommandRunner {
  constructor(
    private readonly initUsecases: InitCliUsecases,
    readonly contextService: ContextService,
  ) {
    super();
  }

  async run(passedParams: string[]): Promise<void> {
    const engine = passedParams[0];
    const options = passedParams.includes(Engines.llamaCPP)
      ? await this.initUsecases.defaultInstallationOptions()
      : {};
    return this.initUsecases
      .installEngine(
        options,
        engine.includes('@') ? engine.split('@')[1] : 'latest',
        engine,
        true
      )
      .then(() => console.log('Engine installed successfully!'))
      .catch(() => console.error('Engine not found or installation failed!'));
  }
}
