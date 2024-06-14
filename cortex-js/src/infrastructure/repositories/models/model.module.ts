import { Module } from '@nestjs/common';
import { CortexProviderModule } from '@/infrastructure/providers/cortex/cortex.module';
import { HttpModule } from '@nestjs/axios';
import { ModelRepository } from '@/domain/repositories/model.interface';
import { ModelRepositoryImpl } from './model.repository';
import { FileManagerModule } from '@/infrastructure/services/file-manager/file-manager.module';
import { DownloadManagerModule } from '@/download-manager/download-manager.module';

@Module({
  imports: [
    CortexProviderModule,
    HttpModule,
    FileManagerModule,
    DownloadManagerModule,
  ],
  providers: [
    {
      provide: ModelRepository,
      useClass: ModelRepositoryImpl,
    },
  ],
  exports: [ModelRepository],
})
export class ModelRepositoryModule {}
