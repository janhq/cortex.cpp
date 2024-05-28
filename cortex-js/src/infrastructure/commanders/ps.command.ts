import { CommandRunner, SubCommand } from 'nest-commander';
import { PSCliUsecases } from './usecases/ps.cli.usecases';

@SubCommand({
  name: 'ps',
  description: 'Show running models and their status',
})
export class PSCommand extends CommandRunner {
  constructor(private readonly usecases: PSCliUsecases) {
    super();
  }
  async run(): Promise<void> {
    this.usecases.getModels();
  }
}
