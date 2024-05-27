import { Module } from '@nestjs/common';
import { InitCliUsecases } from './init.cli.usecases';
import { HttpModule } from '@nestjs/axios';
import { ModelsCliUsecases } from './models.cli.usecases';
import { ModelsModule } from '@/usecases/models/models.module';
import { ChatCliUsecases } from './chat.cli.usecases';
import { ChatModule } from '@/usecases/chat/chat.module';
import { CortexModule } from '@/usecases/cortex/cortex.module';

@Module({
  imports: [HttpModule, ModelsModule, ChatModule, CortexModule],
  providers: [InitCliUsecases, ModelsCliUsecases, ChatCliUsecases],
  exports: [InitCliUsecases, ModelsCliUsecases, ChatCliUsecases],
})
export class CliUsecasesModule {}
