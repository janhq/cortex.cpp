import { Module } from '@nestjs/common';
import { TelemetryUsecases } from './telemetry.usecases';
import { HttpModule } from '@nestjs/axios';
import { FileManagerModule } from '@/file-manager/file-manager.module';
import { DatabaseModule } from '@/infrastructure/database/database.module';
import { UtilModule } from '@/util/util.module';

@Module({
  imports: [HttpModule, DatabaseModule, FileManagerModule, UtilModule],
  providers: [TelemetryUsecases],
  exports: [TelemetryUsecases],
})
export class TelemetryModule {}
