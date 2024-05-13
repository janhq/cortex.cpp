import { Module } from '@nestjs/common';
import { ModelsUsecases } from './models.usecases';
import { ModelsController } from '@/infrastructure/controllers/models.controller';
import { DatabaseModule } from '@/infrastructure/database/database.module';
import { CortexModule } from '@/usecases/cortex/cortex.module';
import { ExtensionModule } from '@/infrastructure/repositories/extensions/extension.module';
import { HttpModule } from '@nestjs/axios';

@Module({
  imports: [DatabaseModule, CortexModule, ExtensionModule, HttpModule],
  controllers: [ModelsController],
  providers: [ModelsUsecases],
  exports: [ModelsUsecases],
})
export class ModelsModule {}
