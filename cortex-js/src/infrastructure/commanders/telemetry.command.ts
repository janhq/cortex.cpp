import { SubCommand, Option } from 'nest-commander';
import { TelemetryUsecases } from '@/usecases/telemetry/telemetry.usecases';
import { TelemetryOptions } from './types/telemetry-options.interface';
import { SetCommandContext } from './decorators/CommandContext';
import { BaseCommand } from './base.command';
import { CortexUsecases } from '@/usecases/cortex/cortex.usecases';

@SubCommand({
  name: 'telemetry',
  description: 'Get telemetry logs',
})
@SetCommandContext()
export class TelemetryCommand extends BaseCommand {
  constructor(
    private readonly telemetryUseCase: TelemetryUsecases,
    readonly cortexUseCases: CortexUsecases,
  ) {
    super(cortexUseCases);
  }

  async runCommand(
    _input: string[],
    options?: TelemetryOptions,
  ): Promise<void> {
    if (options?.type === 'crash') {
      try {
        await this.telemetryUseCase.readCrashReports((telemetryEvent) => {
          console.log(JSON.stringify(telemetryEvent, null, 2));
        });
      } catch (e) {
        console.error('Error reading crash reports', e);
      }
    }
  }
  @Option({
    flags: '-t, --type',
    description: 'Type of telemetry',
    defaultValue: 'crash',
    choices: ['crash'],
  })
  parseType(value: string) {
    return value;
  }
}
