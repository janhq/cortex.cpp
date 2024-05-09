import { Module } from '@nestjs/common';
import { ExtensionRepositoryImpl } from './extension.repository';
import { ExtensionRepository } from '@/domain/repositories/extension.interface';
import { CortexProviderModule } from '@/infrastructure/providers/cortex/cortex.module';

@Module({
  imports: [CortexProviderModule],
  providers: [
    {
      provide: ExtensionRepository,
      useClass: ExtensionRepositoryImpl,
    },
  ],
  exports: [ExtensionRepository],
})
export class ExtensionModule {}
