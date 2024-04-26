import { Test, TestingModule } from '@nestjs/testing';
import { ModelsService } from './models.service';

describe('ModelsService', () => {
  let service: ModelsService;

  beforeEach(async () => {
    const module: TestingModule = await Test.createTestingModule({
      providers: [ModelsService],
    }).compile();

    service = module.get<ModelsService>(ModelsService);
  });

  it('should be defined', () => {
    expect(service).toBeDefined();
  });
});
