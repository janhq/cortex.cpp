console.time('import-command-models-module');
import { Module } from '@nestjs/common';
console.timeEnd('import-command-models-module');
console.time('import-command-models-model');
import { ModelsUsecases } from './models.usecases';
console.timeEnd('import-command-models-model');
console.time('import-command-models-database');
import { DatabaseModule } from '@/infrastructure/database/database.module';
console.timeEnd('import-command-models-database');
console.time('import-command-models-cortex');
import { CortexModule } from '@/usecases/cortex/cortex.module';
console.timeEnd('import-command-models-cortex');
console.time('import-command-models-extension');
import { ExtensionModule } from '@/infrastructure/repositories/extensions/extension.module';
console.timeEnd('import-command-models-extension');
console.time('import-command-models-http');
import { HttpModule } from '@nestjs/axios';
console.timeEnd('import-command-models-http');
console.time('import-command-models-telemetry');
import { TelemetryModule } from '../telemetry/telemetry.module';
console.timeEnd('import-command-models-telemetry');
console.time('import-command-models-file-manager');
import { FileManagerModule } from '@/infrastructure/services/file-manager/file-manager.module';
console.timeEnd('import-command-models-file-manager');
console.time('import-command-models-model-repository');
import { ModelRepositoryModule } from '@/infrastructure/repositories/models/model.module';
console.timeEnd('import-command-models-model-repository');
console.time('import-command-models-download-manager');
import { DownloadManagerModule } from '@/infrastructure/services/download-manager/download-manager.module';
console.timeEnd('import-command-models-download-manager');
console.time('import-command-models-context');
import { ContextModule } from '@/infrastructure/services/context/context.module';
console.timeEnd('import-command-models-context');

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
