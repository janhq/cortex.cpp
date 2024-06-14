import { CommandRunner, SubCommand } from 'nest-commander';
import { ModelsCliUsecases } from '@commanders/usecases/models.cli.usecases';
import { exit } from 'node:process';
import { ContextService } from '@/util/context.service';
import { SetCommandContext } from '../decorators/CommandContext';

@SubCommand({ name: 'remove', description: 'Remove a model by ID locally.' })
@SetCommandContext()
export class ModelRemoveCommand extends CommandRunner {
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

    await this.modelsCliUsecases.removeModel(input[0]).then(console.log);
  }
}
