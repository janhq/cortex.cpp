import { FileManagerService } from '@/infrastructure/services/file-manager/file-manager.service';
import { readdirSync } from 'fs';
import { SubCommand } from 'nest-commander';
import { join } from 'path';
import { SetCommandContext } from './decorators/CommandContext';
import { ContextService } from '../services/context/context.service';
import { CortexUsecases } from '@/usecases/cortex/cortex.usecases';
import { BaseCommand } from './base.command';

@SubCommand({
  name: 'presets',
  description: 'Show all available presets',
})
@SetCommandContext()
export class PresetCommand extends BaseCommand {
  constructor(
    private readonly fileService: FileManagerService,
    readonly contextService: ContextService,
    readonly cortexUsecases: CortexUsecases,
  ) {
    super(cortexUsecases, fileService);
  }
  async runCommand(): Promise<void> {
    return console.table(
      readdirSync(
        join(await this.fileService.getDataFolderPath(), `presets`),
      ).map((e) => ({
        preset: e.replace('.yaml', ''),
      })),
    );
  }
}
