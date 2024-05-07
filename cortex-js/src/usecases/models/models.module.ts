import { Module } from '@nestjs/common';
import { ModelsUsecases } from './models.usecases';
import { ModelsController } from '../../infrastructure/controllers/models.controller';
import { DatabaseModule } from 'src/infrastructure/database/database.module';
import { CortexModule } from '../cortex/cortex.module';
import { ExtensionModule } from '@/infrastructure/repositories/extensions/extension.module';

@Module({
  imports: [DatabaseModule, CortexModule, ExtensionModule],
  controllers: [ModelsController],
  providers: [ModelsUsecases],
  exports: [ModelsUsecases],
})
export class ModelsModule {}
