import { Test, TestingModule } from '@nestjs/testing';
import { ModelsUsecases } from './models.usecases';
import { DatabaseModule } from '@/infrastructure/database/database.module';
import { ModelsModule } from './models.module';
import { ExtensionModule } from '@/infrastructure/repositories/extensions/extension.module';
import { FileManagerModule } from '@/infrastructure/services/file-manager/file-manager.module';
import { HttpModule } from '@nestjs/axios';
import { ModelRepositoryModule } from '@/infrastructure/repositories/models/model.module';
import { DownloadManagerModule } from '@/download-manager/download-manager.module';
import { EventEmitterModule } from '@nestjs/event-emitter';

describe('ModelsService', () => {
  let service: ModelsUsecases;

  beforeEach(async () => {
    const module: TestingModule = await Test.createTestingModule({
      imports: [
        DatabaseModule,
        ModelsModule,
        ExtensionModule,
        FileManagerModule,
        HttpModule,
        ModelRepositoryModule,
        DownloadManagerModule,
        EventEmitterModule.forRoot(),
      ],
      providers: [ModelsUsecases],
      exports: [ModelsUsecases],
    }).compile();

    service = module.get<ModelsUsecases>(ModelsUsecases);
  });

  it('should be defined', () => {
    expect(service).toBeDefined();
  });
});
