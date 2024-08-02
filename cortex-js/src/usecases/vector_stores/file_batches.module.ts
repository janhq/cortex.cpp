import { Module } from '@nestjs/common';
import { DatabaseModule } from '@/infrastructure/database/database.module';
import { ExtensionModule } from '@/infrastructure/repositories/extensions/extension.module';
import { FileManagerModule } from '@/infrastructure/services/file-manager/file-manager.module';
import { FileBatchesUsecases } from './file_batches.usecases';

@Module({
  imports: [ExtensionModule, DatabaseModule, FileManagerModule],
  controllers: [],
  providers: [FileBatchesUsecases],
  exports: [FileBatchesUsecases],
})
export class FileBatchesModule {}
