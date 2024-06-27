import { Module } from '@nestjs/common';
import { TelemetryUsecases } from './telemetry.usecases';
import { HttpModule } from '@nestjs/axios';
import { DatabaseModule } from '@/infrastructure/database/database.module';
import { FileManagerModule } from '@/infrastructure/services/file-manager/file-manager.module';
import { ContextModule } from '@/infrastructure/services/context/context.module';

@Module({
  imports: [HttpModule, DatabaseModule, FileManagerModule, ContextModule],
  providers: [TelemetryUsecases],
  exports: [TelemetryUsecases],
})
export class TelemetryModule {}
