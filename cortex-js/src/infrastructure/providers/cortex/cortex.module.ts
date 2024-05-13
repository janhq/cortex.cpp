import { Module } from '@nestjs/common';
import CortexProvider from './cortex.provider';
import { HttpModule } from '@nestjs/axios';

@Module({
  imports: [HttpModule],
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
