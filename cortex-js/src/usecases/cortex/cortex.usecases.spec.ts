import { Test, TestingModule } from '@nestjs/testing';
import { CortexUsecases } from './cortex.usecases';
import { DatabaseModule } from '@/infrastructure/database/database.module';
import { HttpModule } from '@nestjs/axios';
import { FileManagerModule } from '@/infrastructure/services/file-manager/file-manager.module';

describe('CortexUsecases', () => {
  let cortexUsecases: CortexUsecases;

  beforeEach(async () => {
    const module: TestingModule = await Test.createTestingModule({
      imports: [DatabaseModule, HttpModule, FileManagerModule],
      providers: [CortexUsecases],
      exports: [],
    }).compile();

    cortexUsecases = module.get<CortexUsecases>(CortexUsecases);
  });

  it('should be defined', () => {
    expect(cortexUsecases).toBeDefined();
  });
});
