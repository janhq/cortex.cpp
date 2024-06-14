import { FileManagerService } from '@/file-manager/file-manager.service';
import { TelemetryRepositoryImpl } from '@/infrastructure/repositories/telemetry/telemetry.repository';
// import { HttpService } from '@nestjs/axios';

export const telemetryProviders = [
  {
    provide: 'TELEMETRY_REPOSITORY',
    useFactory: (
      fileManagerService: FileManagerService,
      // httpService: HttpService,
    ) => new TelemetryRepositoryImpl(fileManagerService),
    inject: [FileManagerService],
  },
];
