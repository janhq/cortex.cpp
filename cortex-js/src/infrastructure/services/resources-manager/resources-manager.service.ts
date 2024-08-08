import osUtils from 'node-os-utils'
import {
  ResourceStatus,
  UsedMemInfo,
} from '@/domain/models/resource.interface';
import { getMemoryInformation, MemoryInformation } from '@/utils/system-resource';
import { Injectable } from '@nestjs/common';
import systemInformation, { Systeminformation } from 'systeminformation';

@Injectable()
export class ResourcesManagerService {
  async getResourceStatuses(): Promise<ResourceStatus> {
    const promises = [systemInformation.currentLoad(), getMemoryInformation()];
    const results = await Promise.all(promises);

    const cpuUsage = results[0] as Systeminformation.CurrentLoadData;
    const memory = results[1] as MemoryInformation
    const memInfo: UsedMemInfo = {
      total: memory.total,
      used: memory.used,
    };

    return {
      mem: memInfo,
      cpu: {
        usage: Number(cpuUsage.currentLoad.toFixed(2)),
      },
    };
  }
}
