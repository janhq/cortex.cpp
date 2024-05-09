import { Test, TestingModule } from '@nestjs/testing';
import { InferenceSettingsController } from './inference-settings.controller';
import { InferenceSettingsUsecases } from '@/usecases/inference-settings/inference-settings.usecases';

describe('InferenceSettingsController', () => {
  let controller: InferenceSettingsController;

  beforeEach(async () => {
    const module: TestingModule = await Test.createTestingModule({
      controllers: [InferenceSettingsController],
      providers: [InferenceSettingsUsecases],
    }).compile();

    controller = module.get<InferenceSettingsController>(
      InferenceSettingsController,
    );
  });

  it('should be defined', () => {
    expect(controller).toBeDefined();
  });
});
