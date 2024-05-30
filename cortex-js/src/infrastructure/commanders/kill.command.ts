import { CommandRunner, SubCommand } from 'nest-commander';
import { CortexUsecases } from '@/usecases/cortex/cortex.usecases';

@SubCommand({
  name: 'kill',
  description: 'Kill running cortex processes',
})
export class KillCommand extends CommandRunner {
  constructor(private readonly usecases: CortexUsecases) {
    super();
  }
  async run(): Promise<void> {
    this.usecases.stopCortex().then(console.log);
  }
}
