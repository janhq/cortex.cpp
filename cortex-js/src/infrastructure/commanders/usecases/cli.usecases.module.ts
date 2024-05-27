import { Module } from '@nestjs/common';
import { InitCliUsecases } from './init.cli.usecases';
import { HttpModule } from '@nestjs/axios';
import { ModelsCliUsecases } from './models.cli.usecases';
import { ModelsModule } from '@/usecases/models/models.module';
import { ChatCliUsecases } from './chat.cli.usecases';
import { ChatModule } from '@/usecases/chat/chat.module';
import { CortexModule } from '@/usecases/cortex/cortex.module';
import { ThreadsModule } from '@/usecases/threads/threads.module';
import { AssistantsModule } from '@/usecases/assistants/assistants.module';
import { MessagesModule } from '@/usecases/messages/messages.module';
import { FileManagerModule } from '@/file-manager/file-manager.module';

@Module({
  imports: [
    HttpModule,
    ModelsModule,
    ChatModule,
    CortexModule,
    ThreadsModule,
    AssistantsModule,
    MessagesModule,
    FileManagerModule,
  ],
  providers: [InitCliUsecases, ModelsCliUsecases, ChatCliUsecases],
  exports: [InitCliUsecases, ModelsCliUsecases, ChatCliUsecases],
})
export class CliUsecasesModule {}
