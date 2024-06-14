import { CommandRunner, SubCommand } from 'nest-commander';
import { exit } from 'node:process';
import { ModelsCliUsecases } from '../usecases/models.cli.usecases';
import { SetCommandContext } from '../decorators/CommandContext';
import { ContextService } from '@/util/context.service';

@SubCommand({ name: 'stop', description: 'Stop a model by ID.' })
@SetCommandContext()
export class ModelStopCommand extends CommandRunner {
  constructor(
    private readonly modelsCliUsecases: ModelsCliUsecases,
    readonly contextService: ContextService,
  ) {
    super();
  }

  async run(input: string[]): Promise<void> {
    if (input.length === 0) {
      console.error('Model ID is required');
      exit(1);
    }

    await this.modelsCliUsecases.stopModel(input[0]).then(console.log);
  }
}
