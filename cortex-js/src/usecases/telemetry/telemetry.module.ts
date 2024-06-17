import { Module } from '@nestjs/common';
import { TelemetryUsecases } from './telemetry.usecases';
import { HttpModule } from '@nestjs/axios';
import { DatabaseModule } from '@/infrastructure/database/database.module';
import { UtilModule } from '@/util/util.module';
import { FileManagerModule } from '@/infrastructure/services/file-manager/file-manager.module';

@Module({
  imports: [HttpModule, DatabaseModule, FileManagerModule, UtilModule],
  providers: [TelemetryUsecases],
  exports: [TelemetryUsecases],
})
export class TelemetryModule {}
