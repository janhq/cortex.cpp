import { Test, TestingModule } from '@nestjs/testing';
import { CortexController } from './cortex.controller';

describe('CortexController', () => {
  let controller: CortexController;

  beforeEach(async () => {
    const module: TestingModule = await Test.createTestingModule({
      controllers: [CortexController],
    }).compile();

    controller = module.get<CortexController>(CortexController);
  });

  it('should be defined', () => {
    expect(controller).toBeDefined();
  });
});
