import { FileManagerService } from '@/file-manager/file-manager.service';
import { readdirSync } from 'fs';
import { CommandRunner, SubCommand } from 'nest-commander';
import { join } from 'path';
import { SetCommandContext } from './decorators/CommandContext';
import { ContextService } from '@/util/context.service';

@SubCommand({
  name: 'presets',
  description: 'Show all available presets',
})
@SetCommandContext()
export class PresetCommand extends CommandRunner {
  constructor(
    private readonly fileService: FileManagerService,
    readonly contextService: ContextService,
  ) {
    super();
  }
  async run(): Promise<void> {
    return console.table(
      readdirSync(
        join(await this.fileService.getDataFolderPath(), `presets`),
      ).map((e) => ({
        preset: e.replace('.yaml', ''),
      })),
    );
  }
}
