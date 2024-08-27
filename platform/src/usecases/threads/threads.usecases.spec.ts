import { Test, TestingModule } from '@nestjs/testing';
import { ThreadsUsecases } from './threads.usecases';
import { DatabaseModule } from '@/infrastructure/database/database.module';

describe('ThreadsService', () => {
  let service: ThreadsUsecases;

  beforeEach(async () => {
    const module: TestingModule = await Test.createTestingModule({
      imports: [DatabaseModule],
      providers: [ThreadsUsecases],
    }).compile();

    service = module.get<ThreadsUsecases>(ThreadsUsecases);
  });

  it('should be defined', () => {
    expect(service).toBeDefined();
  });
});
