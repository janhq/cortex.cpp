import { Test, TestingModule } from '@nestjs/testing';
import { AssistantsUsecases } from './assistants.usecases';
import { DatabaseModule } from '@/infrastructure/database/database.module';

describe('AssistantsService', () => {
  let service: AssistantsUsecases;

  beforeEach(async () => {
    const module: TestingModule = await Test.createTestingModule({
      imports: [DatabaseModule],
      exports: [AssistantsUsecases],
      providers: [AssistantsUsecases],
    }).compile();

    service = module.get<AssistantsUsecases>(AssistantsUsecases);
  });

  it('should be defined', () => {
    expect(service).toBeDefined();
  });
});
