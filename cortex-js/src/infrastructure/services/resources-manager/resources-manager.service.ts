import {
  ResourceStatus,
  UsedMemInfo,
} from '@/domain/models/resource.interface';
import { Injectable } from '@nestjs/common';
import { cpu, mem } from 'node-os-utils';

@Injectable()
export class ResourcesManagerService {
  async getResourceStatuses(): Promise<ResourceStatus> {
    const promises = [cpu.usage(), mem.used()];
    const results = await Promise.all(promises);

    const cpuUsage = results[0] as number;
    const memInfo = results[1] as UsedMemInfo;

    return {
      mem: memInfo,
      cpu: {
        usage: cpuUsage,
      },
    };
  }
}
