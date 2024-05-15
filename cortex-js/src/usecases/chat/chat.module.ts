import { Module } from '@nestjs/common';
import { ChatController } from '@/infrastructure/controllers/chat.controller';
import { ChatUsecases } from './chat.usecases';
import { DatabaseModule } from '@/infrastructure/database/database.module';
import { ExtensionModule } from '@/infrastructure/repositories/extensions/extension.module';

@Module({
  imports: [DatabaseModule, ExtensionModule],
  controllers: [ChatController],
  providers: [ChatUsecases],
  exports: [ChatUsecases],
})
export class ChatModule {}
