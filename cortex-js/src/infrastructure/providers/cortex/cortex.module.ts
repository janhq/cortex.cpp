import { Module } from '@nestjs/common';
import CortexProvider from './cortex.provider';

@Module({
  providers: [
    {
      provide: 'CORTEX_PROVIDER',
      useValue: CortexProvider,
    },
  ],
  exports: [
    {
      provide: 'CORTEX_PROVIDER',
      useValue: CortexProvider,
    },
  ],
})
export class CortexProviderModule {}
