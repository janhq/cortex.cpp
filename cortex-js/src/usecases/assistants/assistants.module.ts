import { Module } from '@nestjs/common';
import { AssistantsController } from '../../infrastructure/controllers/assistants.controller';
import { AssistantsUsecases } from './assistants.usecases';
import { DatabaseModule } from 'src/infrastructure/database/database.module';

@Module({
  imports: [DatabaseModule],
  controllers: [AssistantsController],
  providers: [AssistantsUsecases],
  exports: [AssistantsUsecases],
})
export class AssistantsModule {}
