import { Test, TestingModule } from '@nestjs/testing';
import { DatabaseModule } from '../database/database.module';
import { ExtensionModule } from '../repositories/extensions/extension.module';
import { ModelRepositoryModule } from '../repositories/models/model.module';
import { HttpModule } from '@nestjs/axios';
import { DownloadManagerModule } from '@/infrastructure/services/download-manager/download-manager.module';
import { EventEmitterModule } from '@nestjs/event-emitter';
import { TelemetryModule } from '@/usecases/telemetry/telemetry.module';
import { FileManagerModule } from '../services/file-manager/file-manager.module';
import { ConfigsController } from './configs.controller';
import { ConfigsUsecases } from '@/usecases/configs/configs.usecase';
import { ConfigsModule } from '@/usecases/configs/configs.module';

describe('ConfigsController', () => {
  let controller: ConfigsController;

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
        ConfigsModule,
      ],
      controllers: [ConfigsController],
      providers: [ConfigsUsecases],
    }).compile();

    controller = module.get<ConfigsController>(ConfigsController);
  });

  it('should be defined', () => {
    expect(controller).toBeDefined();
  });
});
