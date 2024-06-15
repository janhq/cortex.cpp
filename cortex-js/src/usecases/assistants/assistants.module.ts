import { Module } from '@nestjs/common';
import { AssistantsUsecases } from './assistants.usecases';
import { DatabaseModule } from '@/infrastructure/database/database.module';

@Module({
  imports: [DatabaseModule],
  controllers: [],
  providers: [AssistantsUsecases],
  exports: [AssistantsUsecases],
})
export class AssistantsModule {}
