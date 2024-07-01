import { CommandRunner, SubCommand } from 'nest-commander';
import { PSCliUsecases } from './usecases/ps.cli.usecases';
import { SetCommandContext } from './decorators/CommandContext';
import { ContextService } from '../services/context/context.service';

@SubCommand({
  name: 'ps',
  description: 'Show running models and their status',
})
// @SetCommandContext()
export class PSCommand extends CommandRunner {
  constructor(
    // private readonly usecases: PSCliUsecases,
    // readonly contextService: ContextService,
  ) {
    super();
  }
  async run(): Promise<void> {
    console.log('Running ps command');
    console.time('ps');
    return;
  }
}
