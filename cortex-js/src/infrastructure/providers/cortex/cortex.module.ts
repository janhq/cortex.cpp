import { Module } from '@nestjs/common';
import CortexProvider from './cortex.provider';

@Module({
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
