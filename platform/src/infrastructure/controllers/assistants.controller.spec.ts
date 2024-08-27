import { Test, TestingModule } from '@nestjs/testing';
import { AssistantsController } from './assistants.controller';
import { AssistantsUsecases } from '@/usecases/assistants/assistants.usecases';
import { DatabaseModule } from '@/infrastructure/database/database.module';
import { ModelRepositoryModule } from '../repositories/models/model.module';
import { DownloadManagerModule } from '../services/download-manager/download-manager.module';
import { EventEmitterModule } from '@nestjs/event-emitter';

describe('AssistantsController', () => {
  let controller: AssistantsController;

  beforeEach(async () => {
    const module: TestingModule = await Test.createTestingModule({
      imports: [
        EventEmitterModule.forRoot(),
        DatabaseModule,
        ModelRepositoryModule,
        DownloadManagerModule,
      ],
      controllers: [AssistantsController],
      providers: [AssistantsUsecases],
      exports: [AssistantsUsecases],
    }).compile();

    controller = module.get<AssistantsController>(AssistantsController);
  });

  it('should be defined', () => {
    expect(controller).toBeDefined();
  });
});
