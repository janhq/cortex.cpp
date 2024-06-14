import { CommandRunner, SubCommand } from 'nest-commander';
import { ModelsCliUsecases } from '@commanders/usecases/models.cli.usecases';
import { exit } from 'node:process';
import { ContextService } from '@/util/context.service';
import { SetCommandContext } from '../decorators/CommandContext';

@SubCommand({ name: 'get', description: 'Get a model by ID.' })
@SetCommandContext()
export class ModelGetCommand extends CommandRunner {
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

    const model = await this.modelsCliUsecases.getModel(input[0]);
    if (!model) console.error('Model not found');
    else console.log(model);
  }
}
