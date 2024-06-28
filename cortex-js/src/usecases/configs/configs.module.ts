import { Module } from '@nestjs/common';
import { FileManagerModule } from '@/infrastructure/services/file-manager/file-manager.module';
import { ConfigsUsecases } from './configs.usecase';
import { ExtensionModule } from '@/infrastructure/repositories/extensions/extension.module';

@Module({
  imports: [FileManagerModule, ExtensionModule],
  controllers: [],
  providers: [ConfigsUsecases],
  exports: [ConfigsUsecases],
})
export class ConfigsModule {}
