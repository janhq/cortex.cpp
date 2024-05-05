import { Test, TestingModule } from '@nestjs/testing';
import { CortexService } from './cortex.service';

describe('CortexService', () => {
  let service: CortexService;

  beforeEach(async () => {
    const module: TestingModule = await Test.createTestingModule({
      providers: [CortexService],
    }).compile();

    service = module.get<CortexService>(CortexService);
  });

  it('should be defined', () => {
    expect(service).toBeDefined();
  });
});
