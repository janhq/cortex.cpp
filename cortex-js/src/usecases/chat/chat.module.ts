import { Module } from '@nestjs/common';
import { ChatController } from '@/infrastructure/controllers/chat.controller';
import { ChatUsecases } from './chat.usecases';
import { DatabaseModule } from '@/infrastructure/database/database.module';
import { ExtensionModule } from '@/infrastructure/repositories/extensions/extension.module';
import { ModelRepositoryModule } from '@/infrastructure/repositories/model/model.module';
import { HttpModule } from '@nestjs/axios';

@Module({
  imports: [DatabaseModule, ExtensionModule, ModelRepositoryModule, HttpModule],
  controllers: [ChatController],
  providers: [ChatUsecases],
  exports: [ChatUsecases],
})
export class ChatModule {}
