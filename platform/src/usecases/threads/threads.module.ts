import { Module } from '@nestjs/common';
import { ThreadsUsecases } from './threads.usecases';
import { DatabaseModule } from '@/infrastructure/database/database.module';

@Module({
  imports: [DatabaseModule],
  controllers: [],
  providers: [ThreadsUsecases],
  exports: [ThreadsUsecases],
})
export class ThreadsModule {}
