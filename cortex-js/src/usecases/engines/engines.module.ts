import { Module } from '@nestjs/common';
import { ConfigsModule } from '../configs/configs.module';
import { EnginesUsecases } from './engines.usecase';
import { ExtensionModule } from '@/infrastructure/repositories/extensions/extension.module';

@Module({
  imports: [ConfigsModule, ExtensionModule],
  controllers: [],
  providers: [EnginesUsecases],
  exports: [EnginesUsecases],
})
export class EnginesModule {}
