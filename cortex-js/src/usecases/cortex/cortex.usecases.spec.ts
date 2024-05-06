import { Test, TestingModule } from '@nestjs/testing';
import { CortexUsecases } from './cortex.usecases';

describe('CortexUsecases', () => {
  let cortexUsecases: CortexUsecases;

  beforeEach(async () => {
    const module: TestingModule = await Test.createTestingModule({
      providers: [CortexUsecases],
    }).compile();

    cortexUsecases = module.get<CortexUsecases>(CortexUsecases);
  });

  it('should be defined', () => {
    expect(cortexUsecases).toBeDefined();
  });
});
