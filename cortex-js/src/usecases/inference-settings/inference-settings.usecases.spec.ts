import { Test, TestingModule } from '@nestjs/testing';
import { InferenceSettingsUsecases } from './inference-settings.usecases';

describe('InferenceSettingsService', () => {
  let service: InferenceSettingsUsecases;

  beforeEach(async () => {
    const module: TestingModule = await Test.createTestingModule({
      providers: [InferenceSettingsUsecases],
    }).compile();

    service = module.get<InferenceSettingsUsecases>(InferenceSettingsUsecases);
  });

  it('should be defined', () => {
    expect(service).toBeDefined();
  });
});
