import { ModelsUsecases } from '@/usecases/models/models.usecases';
import { CommandRunner, SubCommand } from 'nest-commander';
import { PullCommand } from './pull.command';
import { StartCommand } from './start.command';

@SubCommand({ name: 'models', subCommands: [PullCommand, StartCommand] })
export class ModelsCommand extends CommandRunner {
  constructor(private readonly modelsUsecases: ModelsUsecases) {
    super();
  }

  async run(input: string[]): Promise<void> {
    const command = input[0];
    const modelId = input[1];

    if (command !== 'list') {
      if (!modelId) {
        console.log('Model ID is required');
        return;
      }
    }

    switch (command) {
      case 'list':
        this.modelsUsecases.findAll().then(console.log);
        return;
      case 'get':
        this.modelsUsecases.findOne(modelId).then(console.log);
        return;
      case 'remove':
        this.modelsUsecases.remove(modelId).then(console.log);
        return;

      case 'stop':
        return this.modelsUsecases
          .stopModel(modelId)
          .then(console.log)
          .catch(console.error);

      case 'stats':
      case 'fetch':
      case 'build': {
        console.log('Command is not supported yet');
        return;
      }

      default:
        console.error(`Command ${command} is not supported`);
        return;
    }
  }
}
