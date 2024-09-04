import { Module } from '@nestjs/common';
import { AssistantsUsecases } from './assistants.usecases';
import { DatabaseModule } from '@/infrastructure/database/database.module';
import { ModelRepositoryModule } from '@/infrastructure/repositories/models/model.module';

@Module({
  imports: [DatabaseModule, ModelRepositoryModule],
  controllers: [],
  providers: [AssistantsUsecases],
  exports: [AssistantsUsecases],
})
export class AssistantsModule {}
