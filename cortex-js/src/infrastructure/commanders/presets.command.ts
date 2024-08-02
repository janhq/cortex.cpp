import { readdirSync } from 'fs';
import { SubCommand } from 'nest-commander';
import { join } from 'path';
import { SetCommandContext } from './decorators/CommandContext';
import { ContextService } from '../services/context/context.service';
import { CortexUsecases } from '@/usecases/cortex/cortex.usecases';
import { BaseCommand } from './base.command';
import { fileManagerService } from '../services/file-manager/file-manager.service';

@SubCommand({
  name: 'presets',
  description: 'Show all available presets',
})
@SetCommandContext()
export class PresetCommand extends BaseCommand {
  constructor(
    readonly contextService: ContextService,
    readonly cortexUsecases: CortexUsecases,
  ) {
    super(cortexUsecases);
  }
  async runCommand(): Promise<void> {
    return console.table(
      readdirSync(
        join(await fileManagerService.getDataFolderPath(), `presets`),
      ).map((e) => ({
        preset: e.replace('.yaml', ''),
      })),
    );
  }
}
