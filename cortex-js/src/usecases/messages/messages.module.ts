import { Module } from '@nestjs/common';
import { MessagesUsecases } from './messages.usecases';
import { MessagesController } from '@/infrastructure/controllers/messages.controller';
import { DatabaseModule } from '@/infrastructure/database/database.module';

@Module({
  imports: [DatabaseModule],
  controllers: [MessagesController],
  providers: [MessagesUsecases],
  exports: [MessagesUsecases],
})
export class MessagesModule {}
