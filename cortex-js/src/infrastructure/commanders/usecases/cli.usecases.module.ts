import { Module } from '@nestjs/common';
import { HttpModule } from '@nestjs/axios';
import { ModelsModule } from '@/usecases/models/models.module';
import { ChatCliUsecases } from './chat.cli.usecases';
import { ChatModule } from '@/usecases/chat/chat.module';
import { CortexModule } from '@/usecases/cortex/cortex.module';
import { ThreadsModule } from '@/usecases/threads/threads.module';
import { AssistantsModule } from '@/usecases/assistants/assistants.module';
import { MessagesModule } from '@/usecases/messages/messages.module';
import { FileManagerModule } from '@/infrastructure/services/file-manager/file-manager.module';
import { PSCliUsecases } from './ps.cli.usecases';
import { TelemetryModule } from '@/usecases/telemetry/telemetry.module';
import { DownloadManagerModule } from '@/infrastructure/services/download-manager/download-manager.module';
import { EnginesModule } from '@/usecases/engines/engines.module';

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
    TelemetryModule,
    DownloadManagerModule,
    EnginesModule,
  ],
  providers: [ChatCliUsecases, PSCliUsecases],
  exports: [ChatCliUsecases, PSCliUsecases],
})
export class CliUsecasesModule {}
