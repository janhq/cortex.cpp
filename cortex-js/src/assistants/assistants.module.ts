import { Module } from '@nestjs/common';
import { AssistantsController } from './assistants.controller';
import { AssistantsService } from './assistants.service';

@Module({
  controllers: [AssistantsController],
  providers: [AssistantsService],
})
export class AssistantsModule {}
