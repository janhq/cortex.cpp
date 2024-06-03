import { Test, TestingModule } from '@nestjs/testing';
import { MessagesUsecases } from './messages.usecases';
import { DatabaseModule } from '@/infrastructure/database/database.module';

describe('MessagesService', () => {
  let service: MessagesUsecases;

  beforeEach(async () => {
    const module: TestingModule = await Test.createTestingModule({
      imports: [DatabaseModule],
      providers: [MessagesUsecases],
      exports: [MessagesUsecases],
    }).compile();

    service = module.get<MessagesUsecases>(MessagesUsecases);
  });

  it('should be defined', () => {
    expect(service).toBeDefined();
  });
});
