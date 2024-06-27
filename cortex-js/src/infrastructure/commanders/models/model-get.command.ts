import { CommandRunner, SubCommand } from 'nest-commander';
import { ModelsCliUsecases } from '@commanders/usecases/models.cli.usecases';
import { exit } from 'node:process';
import { SetCommandContext } from '../decorators/CommandContext';
import { ContextService } from '@/infrastructure/services/context/context.service';

@SubCommand({
  name: 'get',
  description: 'Get a model by ID.',
  arguments: '<model_id>',
  argsDescription: {
    model_id: 'Model ID to get information about.',
  },
})
@SetCommandContext()
export class ModelGetCommand extends CommandRunner {
  constructor(
    private readonly modelsCliUsecases: ModelsCliUsecases,
    readonly contextService: ContextService,
  ) {
    super();
  }

  async run(passedParams: string[]): Promise<void> {
    if (passedParams.length === 0) {
      console.error('Model ID is required');
      exit(1);
    }

    const model = await this.modelsCliUsecases.getModel(passedParams[0]);
    if (!model) console.error('Model not found');
    else console.log(model);
  }
}
