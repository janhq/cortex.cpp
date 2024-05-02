import { Test, TestingModule } from '@nestjs/testing';
import { InferenceSettingsService } from './inference-settings.service';

describe('InferenceSettingsService', () => {
  let service: InferenceSettingsService;

  beforeEach(async () => {
    const module: TestingModule = await Test.createTestingModule({
      providers: [InferenceSettingsService],
    }).compile();

    service = module.get<InferenceSettingsService>(InferenceSettingsService);
  });

  it('should be defined', () => {
    expect(service).toBeDefined();
  });
});
