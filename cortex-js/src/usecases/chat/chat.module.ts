import { Module } from '@nestjs/common';
import { ChatUsecases } from './chat.usecases';
import { DatabaseModule } from '@/infrastructure/database/database.module';
import { ExtensionModule } from '@/infrastructure/repositories/extensions/extension.module';
import { ModelRepositoryModule } from '@/infrastructure/repositories/models/model.module';
import { HttpModule } from '@nestjs/axios';

@Module({
  imports: [DatabaseModule, ExtensionModule, ModelRepositoryModule, HttpModule],
  controllers: [],
  providers: [ChatUsecases],
  exports: [ChatUsecases],
})
export class ChatModule {}
