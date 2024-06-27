import {
  CommandRunner,
  InquirerService,
  SubCommand,
  Option,
} from 'nest-commander';
import { InitCliUsecases } from './usecases/init.cli.usecases';
import { InitOptions } from './types/init-options.interface';
import { SetCommandContext } from './decorators/CommandContext';
import { TelemetryUsecases } from '@/usecases/telemetry/telemetry.usecases';
import {
  EventName,
  TelemetrySource,
} from '@/domain/telemetry/telemetry.interface';
import { ContextService } from '../services/context/context.service';

@SubCommand({
  name: 'init',
  aliases: ['setup'],
  arguments: '[version]',
  description: "Init settings and download cortex's dependencies",
  argsDescription: {
    version: 'Version of cortex engine',
  },
})
@SetCommandContext()
export class InitCommand extends CommandRunner {
  constructor(
    private readonly inquirerService: InquirerService,
    private readonly initUsecases: InitCliUsecases,
    readonly contextService: ContextService,
    private readonly telemetryUsecases: TelemetryUsecases,
  ) {
    super();
  }

  async run(passedParams: string[], options?: InitOptions): Promise<void> {
    if (options?.silent) {
      const installationOptions =
        await this.initUsecases.defaultInstallationOptions();
      await this.initUsecases.installEngine(installationOptions);
    } else {
      options = await this.inquirerService.ask(
        'init-run-mode-questions',
        options,
      );

      const version = passedParams[0] ?? 'latest';

      await this.initUsecases.installEngine(options, version);
      this.telemetryUsecases.sendEvent(
        [
          {
            name: EventName.INIT,
          },
        ],
        TelemetrySource.CLI,
      );
    }
    console.log('Cortex engines installed successfully!');
  }

  @Option({
    flags: '-s, --silent',
    description: 'Init without asking questions',
    defaultValue: false,
  })
  parseSilent() {
    return true;
  }
}
