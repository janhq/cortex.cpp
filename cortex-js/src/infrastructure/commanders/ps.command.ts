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
    return this.usecases
      .getModels()
      .then(console.table)
      .then(() => this.usecases.isAPIServerOnline())
      .then((isOnline) => {
        if (isOnline) console.log('API server is online');
      });
  }
}
