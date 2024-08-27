import { Module } from '@nestjs/common';
import { DownloadManagerService } from './download-manager.service';
import { HttpModule } from '@nestjs/axios';

@Module({
  imports: [HttpModule],
  providers: [DownloadManagerService],
  exports: [DownloadManagerService],
})
export class DownloadManagerModule {}
