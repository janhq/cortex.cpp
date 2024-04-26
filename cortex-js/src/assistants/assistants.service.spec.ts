import { Test, TestingModule } from '@nestjs/testing';
import { AssistantsService } from './assistants.service';

describe('AssistantsService', () => {
  let service: AssistantsService;

  beforeEach(async () => {
    const module: TestingModule = await Test.createTestingModule({
      providers: [AssistantsService],
    }).compile();

    service = module.get<AssistantsService>(AssistantsService);
  });

  it('should be defined', () => {
    expect(service).toBeDefined();
  });
});
