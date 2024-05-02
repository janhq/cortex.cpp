import { Module } from '@nestjs/common';
import { ChatController } from '../../infrastructure/controllers/chat.controller';
import { ChatUsecases } from './chat.usecases';
import { InferenceSettingsModule } from '../inference-settings/inference-settings.module';
import { ModelsModule } from '../models/models.module';

@Module({
  imports: [ModelsModule, InferenceSettingsModule],
  controllers: [ChatController],
  providers: [ChatUsecases],
})
export class ChatModule {}
