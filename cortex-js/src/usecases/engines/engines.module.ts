import { Module } from '@nestjs/common';
import { ConfigsModule } from '../configs/configs.module';
import { EnginesUsecases } from './engines.usecase';
import { ExtensionModule } from '@/infrastructure/repositories/extensions/extension.module';
import { HttpModule } from '@nestjs/axios';
import { FileManagerModule } from '@/infrastructure/services/file-manager/file-manager.module';
import { DownloadManagerModule } from '@/infrastructure/services/download-manager/download-manager.module';

@Module({
  imports: [
    ConfigsModule,
    ExtensionModule,
    HttpModule,
    FileManagerModule,
    DownloadManagerModule,
    ConfigsModule,
  ],
  controllers: [],
  providers: [EnginesUsecases],
  exports: [EnginesUsecases],
})
export class EnginesModule {}
