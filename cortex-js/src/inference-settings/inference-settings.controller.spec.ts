import { Test, TestingModule } from '@nestjs/testing';
import { InferenceSettingsController } from './inference-settings.controller';
import { InferenceSettingsService } from './inference-settings.service';

describe('InferenceSettingsController', () => {
  let controller: InferenceSettingsController;

  beforeEach(async () => {
    const module: TestingModule = await Test.createTestingModule({
      controllers: [InferenceSettingsController],
      providers: [InferenceSettingsService],
    }).compile();

    controller = module.get<InferenceSettingsController>(InferenceSettingsController);
  });

  it('should be defined', () => {
    expect(controller).toBeDefined();
  });
});
