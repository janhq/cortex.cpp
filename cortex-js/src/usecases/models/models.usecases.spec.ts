import { Test, TestingModule } from '@nestjs/testing';
import { ModelsUsecases } from './models.usecases';

describe('ModelsService', () => {
  let service: ModelsUsecases;

  beforeEach(async () => {
    const module: TestingModule = await Test.createTestingModule({
      providers: [ModelsUsecases],
    }).compile();

    service = module.get<ModelsUsecases>(ModelsUsecases);
  });

  it('should be defined', () => {
    expect(service).toBeDefined();
  });
});
