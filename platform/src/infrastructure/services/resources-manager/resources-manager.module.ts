import { Module } from '@nestjs/common';
import { ResourcesManagerService } from './resources-manager.service';

@Module({
  providers: [ResourcesManagerService],
  exports: [ResourcesManagerService],
})
export class ResourceManagerModule {}
