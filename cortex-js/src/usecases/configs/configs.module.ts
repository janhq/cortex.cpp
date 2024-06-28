import { Module } from '@nestjs/common';
import { FileManagerModule } from '@/infrastructure/services/file-manager/file-manager.module';
import { ConfigsUsecases } from './configs.usecase';

@Module({
  imports: [FileManagerModule],
  controllers: [],
  providers: [ConfigsUsecases],
  exports: [ConfigsUsecases],
})
export class ConfigsModule {}
