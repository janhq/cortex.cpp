import { Test, TestingModule } from '@nestjs/testing';
import { DownloadManagerService } from './download-manager.service';

describe('DownloadManagerService', () => {
  let service: DownloadManagerService;

  beforeEach(async () => {
    const module: TestingModule = await Test.createTestingModule({
      providers: [DownloadManagerService],
    }).compile();

    service = module.get<DownloadManagerService>(DownloadManagerService);
  });

  it('should be defined', () => {
    expect(service).toBeDefined();
  });
});
