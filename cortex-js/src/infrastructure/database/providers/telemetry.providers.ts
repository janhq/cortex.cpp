import { TelemetryRepositoryImpl } from '@/infrastructure/repositories/telemetry/telemetry.repository';
import { FileManagerService } from '@/infrastructure/services/file-manager/file-manager.service';

export const telemetryProviders = [
  {
    provide: 'TELEMETRY_REPOSITORY',
    useFactory: (fileManagerService: FileManagerService) =>
      new TelemetryRepositoryImpl(fileManagerService),
    inject: [FileManagerService],
  },
];
