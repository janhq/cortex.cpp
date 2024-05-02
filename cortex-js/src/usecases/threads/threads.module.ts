import { Module } from '@nestjs/common';
import { ThreadsUsecases } from './threads.usecases';
import { ThreadsController } from '../../infrastructure/controllers/threads.controller';
import { DatabaseModule } from 'src/infrastructure/database/database.module';

@Module({
  imports: [DatabaseModule],
  controllers: [ThreadsController],
  providers: [ThreadsUsecases],
  exports: [ThreadsUsecases],
})
export class ThreadsModule {}
