import { Module } from '@nestjs/common';
import { HttpModule } from '@nestjs/axios';
import { FileManagerModule } from '@/infrastructure/services/file-manager/file-manager.module';
import { PSCliUsecases } from './ps.cli.usecases';

@Module({
  imports: [HttpModule, FileManagerModule],
  providers: [PSCliUsecases],
  exports: [PSCliUsecases],
})
export class CliUsecasesModule {}
