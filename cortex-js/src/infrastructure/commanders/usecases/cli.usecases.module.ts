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
import { FileManagerModule } from '@/infrastructure/services/file-manager/file-manager.module';
import { PSCliUsecases } from './ps.cli.usecases';
import { BenchmarkCliUsecases } from './benchmark.cli.usecases';
import { TelemetryModule } from '@/usecases/telemetry/telemetry.module';

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
  ],
  providers: [
    InitCliUsecases,
    ModelsCliUsecases,
    ChatCliUsecases,
    PSCliUsecases,
    BenchmarkCliUsecases,
  ],
  exports: [
    InitCliUsecases,
    ModelsCliUsecases,
    ChatCliUsecases,
    PSCliUsecases,
    BenchmarkCliUsecases,
  ],
})
export class CliUsecasesModule {}
