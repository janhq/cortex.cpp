import { Module } from '@nestjs/common';
import CortexProvider from './cortex.provider';
import { HttpModule } from '@nestjs/axios';
import { FileManagerModule } from '@/infrastructure/services/file-manager/file-manager.module';

@Module({
  imports: [HttpModule, FileManagerModule],
  providers: [
    {
      provide: 'CORTEX_PROVIDER',
      useClass: CortexProvider,
    },
  ],
  exports: [
    {
      provide: 'CORTEX_PROVIDER',
      useClass: CortexProvider,
    },
  ],
})
export class CortexProviderModule {}
