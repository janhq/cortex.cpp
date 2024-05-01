import { Module } from '@nestjs/common';
import { AssistantsController } from './assistants.controller';
import { AssistantsService } from './assistants.service';
import { DatabaseModule } from 'src/database/database.module';

@Module({
  imports: [DatabaseModule],
  controllers: [AssistantsController],
  providers: [AssistantsService],
  exports: [AssistantsService],
})
export class AssistantsModule {}
