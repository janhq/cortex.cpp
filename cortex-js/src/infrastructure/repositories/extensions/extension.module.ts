import { Module } from '@nestjs/common';
import { ExtensionRepositoryImpl } from './extension.repository';
import { ExtensionRepository } from '@/domain/repositories/extension.interface';
import { CortexProviderModule } from '@/infrastructure/providers/cortex/cortex.module';
import { HttpModule } from '@nestjs/axios';

@Module({
  imports: [CortexProviderModule, HttpModule],
  providers: [
    {
      provide: ExtensionRepository,
      useClass: ExtensionRepositoryImpl,
    },
  ],
  exports: [ExtensionRepository],
})
export class ExtensionModule {}
