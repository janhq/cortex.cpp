import { SubCommand } from 'nest-commander';
import { BaseCommand } from '../base.command';
import { CortexUsecases } from '@/usecases/cortex/cortex.usecases';

@SubCommand({
  name: 'stop',
  description: 'Stop the API server',
})
export class ServeStopCommand extends BaseCommand {
  constructor(private readonly cortexUsecases: CortexUsecases) {
    super(cortexUsecases);
  }
  async runCommand(): Promise<void> {
    return this.cortexUsecases
      .stopApiServer()
      .then(() => console.log('API server stopped'));
  }
}
