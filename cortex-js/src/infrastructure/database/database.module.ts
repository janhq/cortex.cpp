import { Module } from '@nestjs/common';
import { threadProviders } from './providers/thread.providers';
import { sqliteDatabaseProviders } from './sqlite-database.providers';
import { assistantProviders } from './providers/assistant.providers';
import { messageProviders } from './providers/message.providers';
import { FileManagerModule } from '@/infrastructure/services/file-manager/file-manager.module';
import { vectorStoreProviders } from './providers/vector_store.providers';
import { fileProviders } from './providers/file.providers';
import { fileBatchesProviders } from './providers/file_batches.providers';

@Module({
  imports: [FileManagerModule],
  providers: [
    ...sqliteDatabaseProviders,
    ...threadProviders,
    ...assistantProviders,
    ...messageProviders,
    ...vectorStoreProviders,
    ...fileProviders,
    ...fileBatchesProviders,
  ],
  exports: [
    ...threadProviders,
    ...assistantProviders,
    ...messageProviders,
    ...vectorStoreProviders,
    ...fileProviders,
    ...fileBatchesProviders,
  ],
})
export class DatabaseModule {}
