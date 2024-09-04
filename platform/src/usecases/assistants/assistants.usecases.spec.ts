import { Test, TestingModule } from '@nestjs/testing';
import { AssistantsUsecases } from './assistants.usecases';
import { DatabaseModule } from '@/infrastructure/database/database.module';
import { ModelRepositoryModule } from '@/infrastructure/repositories/models/model.module';
import { DownloadManagerModule } from '@/infrastructure/services/download-manager/download-manager.module';
import { EventEmitterModule } from '@nestjs/event-emitter';

describe('AssistantsService', () => {
  let service: AssistantsUsecases;

  beforeEach(async () => {
    const module: TestingModule = await Test.createTestingModule({
      imports: [
        EventEmitterModule.forRoot(),
        DatabaseModule,
        ModelRepositoryModule,
        DownloadManagerModule,
      ],
      exports: [AssistantsUsecases],
      providers: [AssistantsUsecases],
    }).compile();

    service = module.get<AssistantsUsecases>(AssistantsUsecases);
  });

  it('should be defined', () => {
    expect(service).toBeDefined();
  });
});
