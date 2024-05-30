import { Module } from '@nestjs/common';
import { CortexUsecases } from './cortex.usecases';
import { HttpModule } from '@nestjs/axios';
import { FileManagerModule } from '@/file-manager/file-manager.module';

@Module({
  imports: [HttpModule, FileManagerModule],
  providers: [CortexUsecases],
  exports: [CortexUsecases],
})
export class CortexModule {}
