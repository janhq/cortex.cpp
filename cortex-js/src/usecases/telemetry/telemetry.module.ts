import { Module } from '@nestjs/common';
import { TelemetryUsecases } from './telemetry.usecases';
import { HttpModule } from '@nestjs/axios';
import { FileManagerModule } from '@/infrastructure/services/file-manager/file-manager.module';
import { ContextModule } from '@/infrastructure/services/context/context.module';
import { TelemetryRepositoryImpl } from '@/infrastructure/repositories/telemetry/telemetry.repository';
import { FileManagerService } from '@/infrastructure/services/file-manager/file-manager.service';

export const telemetryProvider = {
  provide: 'TELEMETRY_REPOSITORY',
  useFactory: (fileManagerService: FileManagerService) =>
    new TelemetryRepositoryImpl(fileManagerService),
  inject: [FileManagerService],
};

@Module({
  imports: [HttpModule, FileManagerModule, ContextModule],
  providers: [telemetryProvider, TelemetryUsecases],
  exports: [telemetryProvider, TelemetryUsecases],
})
export class TelemetryModule {}
