import { Module } from '@nestjs/common';
import { CortexUsecases } from './cortex.usecases';
import { CortexController } from '@/infrastructure/controllers/cortex.controller';
import { HttpModule } from '@nestjs/axios';

@Module({
  imports: [HttpModule],
  providers: [CortexUsecases],
  controllers: [CortexController],
  exports: [CortexUsecases],
})
export class CortexModule {}
