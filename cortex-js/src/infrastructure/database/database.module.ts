import { Module } from '@nestjs/common';
// import { threadProviders } from './providers/thread.providers';
// import { sqliteDatabaseProviders } from './sqlite-database.providers';
// import { assistantProviders } from './providers/assistant.providers';
// import { messageProviders } from './providers/message.providers';
// import { telemetryProviders } from './providers/telemetry.providers';
// import { FileManagerModule } from '@/infrastructure/services/file-manager/file-manager.module';

@Module({
  imports: [],
  providers: [
    // ...sqliteDatabaseProviders,
    // ...threadProviders,
    // ...assistantProviders,
    // ...messageProviders,
    // ...telemetryProviders,
  ],
  exports: [
    // ...threadProviders,
    // ...assistantProviders,
    // ...messageProviders,
    // ...telemetryProviders,
  ],
})
export class DatabaseModule {}
