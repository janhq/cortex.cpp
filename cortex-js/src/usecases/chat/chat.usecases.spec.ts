import { Test, TestingModule } from '@nestjs/testing';
import { ChatUsecases } from './chat.usecases';

describe('ChatService', () => {
  let service: ChatUsecases;

  beforeEach(async () => {
    const module: TestingModule = await Test.createTestingModule({
      providers: [ChatUsecases],
    }).compile();

    service = module.get<ChatUsecases>(ChatUsecases);
  });

  it('should be defined', () => {
    expect(service).toBeDefined();
  });
});
