import { Test, TestingModule } from '@nestjs/testing';
import { EmbeddingsController } from './embeddings.controller';
import { ChatUsecases } from '@/usecases/chat/chat.usecases';
import { DatabaseModule } from '../database/database.module';
import { ModelRepositoryModule } from '../repositories/model/model.module';
import { ExtensionModule } from '../repositories/extensions/extension.module';
import { HttpModule } from '@nestjs/axios';

describe('EmbeddingsController', () => {
  let controller: EmbeddingsController;

  beforeEach(async () => {
    const module: TestingModule = await Test.createTestingModule({
      imports: [
        DatabaseModule,
        ModelRepositoryModule,
        ExtensionModule,
        HttpModule,
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
