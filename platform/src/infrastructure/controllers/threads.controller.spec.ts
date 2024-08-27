import { Test, TestingModule } from '@nestjs/testing';
import { ThreadsController } from './threads.controller';
import { ThreadsUsecases } from '@/usecases/threads/threads.usecases';
import { DatabaseModule } from '../database/database.module';

describe('ThreadsController', () => {
  let controller: ThreadsController;

  beforeEach(async () => {
    const module: TestingModule = await Test.createTestingModule({
      imports: [DatabaseModule],
      controllers: [ThreadsController],
      providers: [ThreadsUsecases],
    }).compile();

    controller = module.get<ThreadsController>(ThreadsController);
  });

  it('should be defined', () => {
    expect(controller).toBeDefined();
  });
});
