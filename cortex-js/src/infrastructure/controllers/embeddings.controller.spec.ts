import { Test, TestingModule } from '@nestjs/testing';
import { EmbeddingsController } from './embeddings.controller';
import { ChatUsecases } from '@/usecases/chat/chat.usecases';
import { DatabaseModule } from '../database/database.module';
import { ModelRepositoryModule } from '../repositories/models/model.module';
import { ExtensionModule } from '../repositories/extensions/extension.module';
import { HttpModule } from '@nestjs/axios';
import { DownloadManagerModule } from '@/download-manager/download-manager.module';
import { EventEmitterModule } from '@nestjs/event-emitter';
import { TelemetryModule } from '@/usecases/telemetry/telemetry.module';

describe('EmbeddingsController', () => {
  let controller: EmbeddingsController;

  beforeEach(async () => {
    const module: TestingModule = await Test.createTestingModule({
      imports: [
        EventEmitterModule.forRoot(),
        DatabaseModule,
        ModelRepositoryModule,
        ExtensionModule,
        HttpModule,
        DownloadManagerModule,
        EventEmitterModule.forRoot(),
        TelemetryModule,
      ],
      controllers: [EmbeddingsController],
      providers: [ChatUsecases],
      exports: [ChatUsecases],
    }).compile();

    controller = module.get<EmbeddingsController>(EmbeddingsController);
  });

  it('should be defined', () => {
    expect(controller).toBeDefined();
  });
});
