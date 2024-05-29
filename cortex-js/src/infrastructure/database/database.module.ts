import { Module } from '@nestjs/common';
import { threadProviders } from './providers/thread.providers';
import { sqliteDatabaseProviders } from './sqlite-database.providers';
import { modelProviders } from './providers/model.providers';
import { assistantProviders } from './providers/assistant.providers';
import { messageProviders } from './providers/message.providers';
import { FileManagerModule } from '@/file-manager/file-manager.module';

@Module({
  imports: [FileManagerModule],
  providers: [
    ...sqliteDatabaseProviders,
    ...threadProviders,
    ...modelProviders,
    ...assistantProviders,
    ...messageProviders,
  ],
  exports: [
    ...threadProviders,
    ...modelProviders,
    ...assistantProviders,
    ...messageProviders,
  ],
})
export class DatabaseModule {}
