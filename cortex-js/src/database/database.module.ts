import { Module } from '@nestjs/common';
import { threadProviders } from './providers/thread.providers';
import { sqliteDatabaseProviders } from './sqlite-database.providers';
import { modelProviders } from './providers/model.providers';
import { assistantProviders } from './providers/assistant.providers';

@Module({
  providers: [
    ...sqliteDatabaseProviders,
    ...threadProviders,
    ...modelProviders,
    ...assistantProviders,
  ],
  exports: [...threadProviders, ...modelProviders, ...assistantProviders],
})
export class DatabaseModule {}
