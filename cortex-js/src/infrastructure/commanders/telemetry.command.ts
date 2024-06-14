import { CommandRunner, SubCommand, Option } from 'nest-commander';
import { TelemetryUsecases } from '@/usecases/telemetry/telemetry.usecases';
import { TelemetryOptions } from './types/telemetry-options.interface';
import { SetCommandContext } from './decorators/CommandContext';

@SubCommand({
  name: 'telemetry',
  description: 'Get telemetry logs',
})
@SetCommandContext()
export class TelemetryCommand extends CommandRunner {
  constructor(private readonly telemetryUseCase: TelemetryUsecases) {
    super();
  }

  async run(_input: string[], options?: TelemetryOptions): Promise<void> {
    if (options?.type === 'crash') {
      try {
        await this.telemetryUseCase.readCrashReports((telemetryEvent) => {
          const formattedLog = console.log(
            JSON.stringify(telemetryEvent, null, 2),
          );
          console.log(formattedLog);
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
