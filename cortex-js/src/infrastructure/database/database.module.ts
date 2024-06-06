import { Module } from '@nestjs/common';
import { threadProviders } from './providers/thread.providers';
import { sqliteDatabaseProviders } from './sqlite-database.providers';
import { assistantProviders } from './providers/assistant.providers';
import { messageProviders } from './providers/message.providers';
import { FileManagerModule } from '@/file-manager/file-manager.module';

@Module({
  imports: [FileManagerModule],
  providers: [
    ...sqliteDatabaseProviders,
    ...threadProviders,
    ...assistantProviders,
    ...messageProviders,
  ],
  exports: [...threadProviders, ...assistantProviders, ...messageProviders],
})
export class DatabaseModule {}
