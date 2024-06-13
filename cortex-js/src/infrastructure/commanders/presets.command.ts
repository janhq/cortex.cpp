import { FileManagerService } from '@/infrastructure/services/file-manager/file-manager.service';
import { readdirSync } from 'fs';
import { CommandRunner, SubCommand } from 'nest-commander';
import { join } from 'path';

@SubCommand({
  name: 'presets',
  description: 'Show all available presets',
})
export class PresetCommand extends CommandRunner {
  constructor(private readonly fileService: FileManagerService) {
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
