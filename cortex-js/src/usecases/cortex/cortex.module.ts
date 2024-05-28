import { Module } from '@nestjs/common';
import { CortexUsecases } from './cortex.usecases';
import { CortexController } from '@/infrastructure/controllers/cortex.controller';
import { HttpModule } from '@nestjs/axios';
import { FileManagerModule } from '@/file-manager/file-manager.module';

@Module({
  imports: [HttpModule, FileManagerModule],
  providers: [CortexUsecases],
  controllers: [CortexController],
  exports: [CortexUsecases],
})
export class CortexModule {}
