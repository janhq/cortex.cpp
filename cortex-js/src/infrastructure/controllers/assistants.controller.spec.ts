import { Test, TestingModule } from '@nestjs/testing';
import { AssistantsController } from './assistants.controller';
import { AssistantsUsecases } from '@/usecases/assistants/assistants.usecases';
import { DatabaseModule } from '../database/database.module';

describe('AssistantsController', () => {
  let controller: AssistantsController;

  beforeEach(async () => {
    const module: TestingModule = await Test.createTestingModule({
      imports: [DatabaseModule],
      controllers: [AssistantsController],
      providers: [AssistantsUsecases],
      exports: [AssistantsUsecases],
    }).compile();

    controller = module.get<AssistantsController>(AssistantsController);
  });

  it('should be defined', () => {
    expect(controller).toBeDefined();
  });
});
