import { Module } from '@nestjs/common';
import { DatabaseModule } from '@/infrastructure/database/database.module';
import { VectorStoresUsecases } from './vector_stores.usecases';
import { ExtensionModule } from '@/infrastructure/repositories/extensions/extension.module';
import { FileManagerModule } from '@/infrastructure/services/file-manager/file-manager.module';

@Module({
  imports: [ExtensionModule, DatabaseModule, FileManagerModule],
  controllers: [],
  providers: [VectorStoresUsecases],
  exports: [VectorStoresUsecases],
})
export class VectorStoresModule {}
