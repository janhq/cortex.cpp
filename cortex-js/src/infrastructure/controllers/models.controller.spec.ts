import { Test, TestingModule } from '@nestjs/testing';
import { ModelsController } from './models.controller';
import { ModelsUsecases } from '../../usecases/models/models.usecases';

describe('ModelsController', () => {
  let controller: ModelsController;

  beforeEach(async () => {
    const module: TestingModule = await Test.createTestingModule({
      controllers: [ModelsController],
      providers: [ModelsUsecases],
    }).compile();

    controller = module.get<ModelsController>(ModelsController);
  });

  it('should be defined', () => {
    expect(controller).toBeDefined();
  });
});
