import { CommandRunner } from 'nest-commander';
import { Injectable } from '@nestjs/common';
import { CortexUsecases } from '@/usecases/cortex/cortex.usecases';
import ora from 'ora';
@Injectable()
export abstract class BaseCommand extends CommandRunner {
  constructor(readonly cortexUseCases: CortexUsecases) {
    super();
  }
  protected abstract runCommand(
    passedParam: string[],
    options?: Record<string, any>,
  ): Promise<void>;

  async run(
    passedParam: string[],
    options?: Record<string, any>,
  ): Promise<void> {
    const checkingSpinner = ora('Checking API server online...').start();
    const result = await this.cortexUseCases.isAPIServerOnline();
    if (!result) {
      checkingSpinner.fail(
        'API server is offline. Please run "cortex" before running this command',
      );
      process.exit(1);
    }
    checkingSpinner.succeed('API server is online');
    await this.runCommand(passedParam, options);
  }
}
