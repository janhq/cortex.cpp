import ora from 'ora';
import { CommandRunner, SubCommand } from 'nest-commander';
import { PSCliUsecases } from './usecases/ps.cli.usecases';
import { SetCommandContext } from './decorators/CommandContext';
import { ContextService } from '../services/context/context.service';
import { ModelStat } from './types/model-stat.interface';

@SubCommand({
  name: 'ps',
  description: 'Show running models and their status',
})
@SetCommandContext()
export class PSCommand extends CommandRunner {
  constructor(
    private readonly usecases: PSCliUsecases,
    readonly contextService: ContextService,
  ) {
    super();
  }
  async run(): Promise<void> {
    const runningSpinner = ora('Running PS command...').start();
    let checkingSpinner: ora.Ora
    return this.usecases
      .getModels()
      .then((models: ModelStat[]) => {
        runningSpinner.succeed();
        console.table(models);
      })
      .then(() => {
        checkingSpinner = ora('Checking API server...').start();
        return this.usecases.isAPIServerOnline();
      })
      .then((isOnline) => {
        checkingSpinner.succeed(isOnline ? 'API server is online' : 'API server is offline');
      });
  }
}