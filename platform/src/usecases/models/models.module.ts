import { Module } from '@nestjs/common';
import { ModelsUsecases } from './models.usecases';
import { DatabaseModule } from '@/infrastructure/database/database.module';
import { CortexModule } from '@/usecases/cortex/cortex.module';
import { ExtensionModule } from '@/infrastructure/repositories/extensions/extension.module';
import { HttpModule } from '@nestjs/axios';
import { TelemetryModule } from '../telemetry/telemetry.module';
import { FileManagerModule } from '@/infrastructure/services/file-manager/file-manager.module';
import { ModelRepositoryModule } from '@/infrastructure/repositories/models/model.module';
import { DownloadManagerModule } from '@/infrastructure/services/download-manager/download-manager.module';
import { ContextModule } from '@/infrastructure/services/context/context.module';

@Module({
  imports: [
    DatabaseModule,
    CortexModule,
    ExtensionModule,
    HttpModule,
    FileManagerModule,
    TelemetryModule,
    ContextModule,
    ModelRepositoryModule,
    DownloadManagerModule,
  ],
  controllers: [],
  providers: [ModelsUsecases],
  exports: [ModelsUsecases],
})
export class ModelsModule {}
