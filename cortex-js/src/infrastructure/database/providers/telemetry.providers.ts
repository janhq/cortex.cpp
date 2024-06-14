import { TelemetryRepositoryImpl } from '@/infrastructure/repositories/telemetry/telemetry.repository';
import { FileManagerService } from '@/infrastructure/services/file-manager/file-manager.service';
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
