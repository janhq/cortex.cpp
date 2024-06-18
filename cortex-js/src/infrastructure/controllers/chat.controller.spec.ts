import { Test, TestingModule } from '@nestjs/testing';
import { ChatController } from './chat.controller';
import { ChatUsecases } from '@/usecases/chat/chat.usecases';
import { DatabaseModule } from '../database/database.module';
import { ExtensionModule } from '../repositories/extensions/extension.module';
import { ModelRepositoryModule } from '../repositories/models/model.module';
import { HttpModule } from '@nestjs/axios';
import { DownloadManagerModule } from '@/download-manager/download-manager.module';
import { EventEmitterModule } from '@nestjs/event-emitter';
import { TelemetryModule } from '@/usecases/telemetry/telemetry.module';

describe('ChatController', () => {
  let controller: ChatController;

  beforeEach(async () => {
    const module: TestingModule = await Test.createTestingModule({
      imports: [
        EventEmitterModule.forRoot(),
        DatabaseModule,
        ExtensionModule,
        ModelRepositoryModule,
        HttpModule,
        DownloadManagerModule,
        EventEmitterModule.forRoot(),
        TelemetryModule,
      ],
      controllers: [ChatController],
      providers: [ChatUsecases],
    }).compile();

    controller = module.get<ChatController>(ChatController);
  });

  it('should be defined', () => {
    expect(controller).toBeDefined();
  });
});
