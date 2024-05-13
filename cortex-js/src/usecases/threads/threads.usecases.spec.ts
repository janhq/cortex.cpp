import { Test, TestingModule } from '@nestjs/testing';
import { ThreadsUsecases } from './threads.usecases';

describe('ThreadsService', () => {
  let service: ThreadsUsecases;

  beforeEach(async () => {
    const module: TestingModule = await Test.createTestingModule({
      providers: [ThreadsUsecases],
    }).compile();

    service = module.get<ThreadsUsecases>(ThreadsUsecases);
  });

  it('should be defined', () => {
    expect(service).toBeDefined();
  });
});
