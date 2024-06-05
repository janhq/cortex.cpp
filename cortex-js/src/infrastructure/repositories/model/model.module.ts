import { Module } from '@nestjs/common';
import { CortexProviderModule } from '@/infrastructure/providers/cortex/cortex.module';
import { HttpModule } from '@nestjs/axios';
import { ModelRepository } from '@/domain/repositories/model.interface';
import { ModelRepositoryImpl } from './model.repository';
import { FileManagerModule } from '@/file-manager/file-manager.module';

@Module({
  imports: [CortexProviderModule, HttpModule, FileManagerModule],
  providers: [
    {
      provide: ModelRepository,
      useClass: ModelRepositoryImpl,
    },
  ],
  exports: [ModelRepository],
})
export class ModelRepositoryModule {}
