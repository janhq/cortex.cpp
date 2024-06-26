import { Test, TestingModule } from '@nestjs/testing';
import { DatabaseModule } from '../database/database.module';
import { ExtensionModule } from '../repositories/extensions/extension.module';
import { ModelRepositoryModule } from '../repositories/models/model.module';
import { HttpModule } from '@nestjs/axios';
import { DownloadManagerModule } from '@/infrastructure/services/download-manager/download-manager.module';
import { EventEmitterModule } from '@nestjs/event-emitter';
import { TelemetryModule } from '@/usecases/telemetry/telemetry.module';
import { FileManagerModule } from '../services/file-manager/file-manager.module';
import { EnginesController } from './engines.controller';
import { EnginesUsecases } from '@/usecases/engines/engines.usecase';
import { EnginesModule } from '@/usecases/engines/engines.module';

describe('ConfigsController', () => {
  let controller: EnginesController;

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
        FileManagerModule,
        EnginesModule,
      ],
      controllers: [EnginesController],
      providers: [EnginesUsecases],
    }).compile();

    controller = module.get<EnginesController>(EnginesController);
  });

  it('should be defined', () => {
    expect(controller).toBeDefined();
  });
});
