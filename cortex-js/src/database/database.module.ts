import { Module } from '@nestjs/common';
import { threadProviders } from './providers/thread.providers';
import { sqliteDatabaseProviders } from './sqlite-database.providers';

@Module({
  providers: [...sqliteDatabaseProviders, ...threadProviders],
  exports: [...threadProviders],
})
export class DatabaseModule {}
