import { Module } from '@nestjs/common';
import { ModelsUsecases } from './models.usecases';
import { ModelsController } from '../../infrastructure/controllers/models.controller';
import { DatabaseModule } from 'src/infrastructure/database/database.module';

@Module({
  imports: [DatabaseModule],
  controllers: [ModelsController],
  providers: [ModelsUsecases],
  exports: [ModelsUsecases],
})
export class ModelsModule {}
