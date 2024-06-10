import { Module } from '@nestjs/common';
import { MessagesUsecases } from './messages.usecases';
import { DatabaseModule } from '@/infrastructure/database/database.module';

@Module({
  imports: [DatabaseModule],
  controllers: [],
  providers: [MessagesUsecases],
  exports: [MessagesUsecases],
})
export class MessagesModule {}
