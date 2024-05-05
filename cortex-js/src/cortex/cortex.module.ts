import { Module } from '@nestjs/common';
import { CortexService } from './cortex.service';
import { CortexController } from './cortex.controller';

@Module({
  providers: [CortexService],
  controllers: [CortexController],
  exports: [CortexService],
})
export class CortexModule {}
