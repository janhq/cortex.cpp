import { Test, TestingModule } from '@nestjs/testing';
import { ModelsController } from './models.controller';
import { ModelsUsecases } from '@/usecases/models/models.usecases';
import { DatabaseModule } from '../database/database.module';
import { ExtensionModule } from '../repositories/extensions/extension.module';
import { FileManagerModule } from '@/file-manager/file-manager.module';
import { HttpModule } from '@nestjs/axios';
import { CortexUsecases } from '@/usecases/cortex/cortex.usecases';

describe('ModelsController', () => {
  let controller: ModelsController;

  beforeEach(async () => {
    const module: TestingModule = await Test.createTestingModule({
      imports: [DatabaseModule, ExtensionModule, FileManagerModule, HttpModule],
      controllers: [ModelsController],
      providers: [ModelsUsecases, CortexUsecases],
      exports: [ModelsUsecases],
    }).compile();

    controller = module.get<ModelsController>(ModelsController);
  });

  it('should be defined', () => {
    expect(controller).toBeDefined();
  });
});
