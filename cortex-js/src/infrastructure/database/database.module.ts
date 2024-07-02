console.time('import-command-database-module-total');
console.time('import-command-database-module-module');
import { Module } from '@nestjs/common';
console.timeEnd('import-command-database-module-module');
console.time('import-command-database-module-thread-providers');
import { threadProviders } from './providers/thread.providers';
console.timeEnd('import-command-database-module-thread-providers');
console.time('import-command-database-module-sqlite-database-providers');
import { sqliteDatabaseProviders } from './sqlite-database.providers';
console.timeEnd('import-command-database-module-sqlite-database-providers');
console.time('import-command-database-module-assistant-providers');
import { assistantProviders } from './providers/assistant.providers';
console.timeEnd('import-command-database-module-assistant-providers');
console.time('import-command-database-module-message-providers');
import { messageProviders } from './providers/message.providers';
console.timeEnd('import-command-database-module-message-providers');
console.time('import-command-database-module-telemetry-providers');
import { telemetryProviders } from './providers/telemetry.providers';
console.timeEnd('import-command-database-module-telemetry-providers');
console.time('import-command-database-module-file-manager-module');
import { FileManagerModule } from '@/infrastructure/services/file-manager/file-manager.module';
console.timeEnd('import-command-database-module-file-manager-module');
console.timeEnd('import-command-database-module-total');

@Module({
  imports: [FileManagerModule],
  providers: [
    ...sqliteDatabaseProviders,
    ...threadProviders,
    ...assistantProviders,
    ...messageProviders,
    ...telemetryProviders,
  ],
  exports: [
    ...threadProviders,
    ...assistantProviders,
    ...messageProviders,
    ...telemetryProviders,
  ],
})
export class DatabaseModule {}
