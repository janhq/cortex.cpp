import { Test, TestingModule } from '@nestjs/testing';
import { DownloadManagerService } from './download-manager.service';
import { HttpModule } from '@nestjs/axios';
import { EventEmitterModule } from '@nestjs/event-emitter';

describe('DownloadManagerService', () => {
  let service: DownloadManagerService;

  beforeEach(async () => {
    const module: TestingModule = await Test.createTestingModule({
      imports: [HttpModule, EventEmitterModule.forRoot()],
      providers: [DownloadManagerService],
    }).compile();

    service = module.get<DownloadManagerService>(DownloadManagerService);
  });

  it('should be defined', () => {
    expect(service).toBeDefined();
  });
});
