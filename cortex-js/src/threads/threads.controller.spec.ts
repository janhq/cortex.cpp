import { Test, TestingModule } from '@nestjs/testing';
import { ThreadsController } from './threads.controller';
import { ThreadsService } from './threads.service';

describe('ThreadsController', () => {
  let controller: ThreadsController;

  beforeEach(async () => {
    const module: TestingModule = await Test.createTestingModule({
      controllers: [ThreadsController],
      providers: [ThreadsService],
    }).compile();

    controller = module.get<ThreadsController>(ThreadsController);
  });

  it('should be defined', () => {
    expect(controller).toBeDefined();
  });
});
