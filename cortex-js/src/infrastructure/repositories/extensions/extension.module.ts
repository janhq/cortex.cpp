import { Module } from '@nestjs/common';
import { ExtensionRepositoryImpl } from './extension.repository';
import { ExtensionRepository } from '@/domain/repositories/extension.interface';
import { CortexProviderModule } from '@/infrastructure/providers/cortex/cortex.module';
import { HttpModule } from '@nestjs/axios';
import { FileManagerModule } from '@/infrastructure/services/file-manager/file-manager.module';
import { ExtensionsModule } from '@/extensions/extensions.module';

@Module({
  imports: [
    CortexProviderModule,
    HttpModule,
    FileManagerModule,
    ExtensionsModule,
  ],
  providers: [
    {
      provide: ExtensionRepository,
      useClass: ExtensionRepositoryImpl,
    },
  ],
  exports: [ExtensionRepository],
})
export class ExtensionModule {}
