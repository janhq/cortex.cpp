import { Test, TestingModule } from '@nestjs/testing';
import { AssistantsController } from './assistants.controller';

describe('AssistantsController', () => {
  let controller: AssistantsController;

  beforeEach(async () => {
    const module: TestingModule = await Test.createTestingModule({
      controllers: [AssistantsController],
    }).compile();

    controller = module.get<AssistantsController>(AssistantsController);
  });

  it('should be defined', () => {
    expect(controller).toBeDefined();
  });
});
