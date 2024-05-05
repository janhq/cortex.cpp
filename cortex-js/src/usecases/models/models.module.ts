import { Module } from '@nestjs/common';
import { ModelsUsecases } from './models.usecases';
import { ModelsController } from '../../infrastructure/controllers/models.controller';
import { DatabaseModule } from 'src/infrastructure/database/database.module';
import { CortexModule } from 'src/cortex/cortex.module';

@Module({
  imports: [DatabaseModule, CortexModule],
  controllers: [ModelsController],
  providers: [ModelsUsecases],
  exports: [ModelsUsecases],
})
export class ModelsModule {}
