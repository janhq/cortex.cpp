import { Test, TestingModule } from '@nestjs/testing';
import { AssistantsUsecases } from './assistants.usecases';

describe('AssistantsService', () => {
  let service: AssistantsUsecases;

  beforeEach(async () => {
    const module: TestingModule = await Test.createTestingModule({
      providers: [AssistantsUsecases],
    }).compile();

    service = module.get<AssistantsUsecases>(AssistantsUsecases);
  });

  it('should be defined', () => {
    expect(service).toBeDefined();
  });
});
