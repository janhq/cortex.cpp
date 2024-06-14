import { Test, TestingModule } from '@nestjs/testing';
import { ChatController } from './chat.controller';
import { ChatUsecases } from '@/usecases/chat/chat.usecases';
import { DatabaseModule } from '../database/database.module';
import { ExtensionModule } from '../repositories/extensions/extension.module';
import { ModelRepositoryModule } from '../repositories/models/model.module';
import { HttpModule } from '@nestjs/axios';

describe('ChatController', () => {
  let controller: ChatController;

  beforeEach(async () => {
    const module: TestingModule = await Test.createTestingModule({
      imports: [
        DatabaseModule,
        ExtensionModule,
        ModelRepositoryModule,
        HttpModule,
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
