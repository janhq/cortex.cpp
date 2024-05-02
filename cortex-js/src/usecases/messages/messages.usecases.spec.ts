import { Test, TestingModule } from '@nestjs/testing';
import { MessagesUsecases } from './messages.usecases';

describe('MessagesService', () => {
  let service: MessagesUsecases;

  beforeEach(async () => {
    const module: TestingModule = await Test.createTestingModule({
      providers: [MessagesUsecases],
    }).compile();

    service = module.get<MessagesUsecases>(MessagesUsecases);
  });

  it('should be defined', () => {
    expect(service).toBeDefined();
  });
});
