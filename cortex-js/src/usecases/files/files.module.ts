import { Module } from '@nestjs/common';
import { FilesUsecases } from './files.usecases';
import { DatabaseModule } from '@/infrastructure/database/database.module';
import { FileManagerModule } from '@/infrastructure/services/file-manager/file-manager.module';

@Module({
  imports: [DatabaseModule, FileManagerModule],
  controllers: [],
  providers: [FilesUsecases],
  exports: [FilesUsecases],
})
export class FilesModule {}
