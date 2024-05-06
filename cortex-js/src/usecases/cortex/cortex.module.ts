import { Module } from '@nestjs/common';
import { CortexUsecases } from './cortex.usecases';
import { CortexController } from 'src/infrastructure/controllers/cortex.controller';

@Module({
  providers: [CortexUsecases],
  controllers: [CortexController],
  exports: [CortexUsecases],
})
export class CortexModule {}
