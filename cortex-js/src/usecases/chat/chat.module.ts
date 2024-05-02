import { Module } from '@nestjs/common';
import { ChatController } from '../../infrastructure/controllers/chat.controller';
import { ChatUsecases } from './chat.usecases';

@Module({
  controllers: [ChatController],
  providers: [ChatUsecases],
})
export class ChatModule {}
