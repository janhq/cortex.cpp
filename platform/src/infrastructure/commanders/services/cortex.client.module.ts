import { Module } from '@nestjs/common';
import { CortexClient } from './cortex.client';

@Module({
  imports: [],
  providers: [CortexClient],
  exports: [CortexClient],
})
export class CortexClientModule {}
