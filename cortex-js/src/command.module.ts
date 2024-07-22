import { Module } from '@nestjs/common';
import { ModelsModule } from './usecases/models/models.module';
import { DatabaseModule } from './infrastructure/database/database.module';
import { ConfigModule } from '@nestjs/config';
import { CortexModule } from './usecases/cortex/cortex.module';
import { ModelsCommand } from './infrastructure/commanders/models.command';
import { ExtensionModule } from './infrastructure/repositories/extensions/extension.module';
import { HttpModule } from '@nestjs/axios';
import { InitRunModeQuestions } from './infrastructure/commanders/questions/init.questions';
import { ModelListCommand } from './infrastructure/commanders/models/model-list.command';
import { ModelPullCommand } from './infrastructure/commanders/models/model-pull.command';
import { CortexCommand } from './infrastructure/commanders/cortex-command.commander';
import { ChatCommand } from './infrastructure/commanders/chat.command';
import { ModelStartCommand } from './infrastructure/commanders/models/model-start.command';
import { ModelStopCommand } from './infrastructure/commanders/models/model-stop.command';
import { ModelGetCommand } from './infrastructure/commanders/models/model-get.command';
import { ModelRemoveCommand } from './infrastructure/commanders/models/model-remove.command';
import { RunCommand } from './infrastructure/commanders/shortcuts/run.command';
import { ModelUpdateCommand } from './infrastructure/commanders/models/model-update.command';
import { AssistantsModule } from './usecases/assistants/assistants.module';
import { MessagesModule } from './usecases/messages/messages.module';
import { FileManagerModule } from './infrastructure/services/file-manager/file-manager.module';
import { PSCommand } from './infrastructure/commanders/ps.command';
import { PresetCommand } from './infrastructure/commanders/presets.command';
import { TelemetryModule } from './usecases/telemetry/telemetry.module';
import { TelemetryCommand } from './infrastructure/commanders/telemetry.command';
import { EmbeddingCommand } from './infrastructure/commanders/embeddings.command';
import { BenchmarkCommand } from './infrastructure/commanders/benchmark.command';
import { EventEmitterModule } from '@nestjs/event-emitter';
import { DownloadManagerModule } from './infrastructure/services/download-manager/download-manager.module';
import { ServeStopCommand } from './infrastructure/commanders/sub-commands/serve-stop.command';
import { ContextModule } from './infrastructure/services/context/context.module';
import { CliUsecasesModule } from './infrastructure/commanders/usecases/cli.usecases.module';
import { ExtensionsModule } from './extensions/extensions.module';
import { EnginesCommand } from './infrastructure/commanders/engines.command';
import { ConfigsModule } from './usecases/configs/configs.module';
import { EnginesModule } from './usecases/engines/engines.module';
import { EnginesListCommand } from './infrastructure/commanders/engines/engines-list.command';
import { EnginesGetCommand } from './infrastructure/commanders/engines/engines-get.command';
import { EnginesInitCommand } from './infrastructure/commanders/engines/engines-init.command';
import { EnginesSetCommand } from './infrastructure/commanders/engines/engines-set.command';

@Module({
  imports: [
    ConfigModule.forRoot({
      isGlobal: true,
      envFilePath:
        process.env.NODE_ENV !== 'production' ? '.env.development' : '.env',
    }),
    EventEmitterModule.forRoot(),
    DatabaseModule,
    ModelsModule,
    CortexModule,
    ExtensionModule,
    HttpModule,
    CliUsecasesModule,
    AssistantsModule,
    MessagesModule,
    FileManagerModule,
    TelemetryModule,
    ContextModule,
    DownloadManagerModule,
    ExtensionsModule,
    ConfigsModule,
    EnginesModule,
  ],
  providers: [
    CortexCommand,
    ModelsCommand,
    ChatCommand,
    PSCommand,
    PresetCommand,
    EmbeddingCommand,
    BenchmarkCommand,
    EnginesCommand,

    // Questions
    InitRunModeQuestions,

    // Model commands
    ModelStartCommand,
    ModelStopCommand,
    ModelListCommand,
    ModelGetCommand,
    ModelRemoveCommand,
    ModelPullCommand,
    ModelUpdateCommand,

    // Shortcuts
    RunCommand,

    // Telemetry
    TelemetryCommand,

    // Serve
    ServeStopCommand,

    // Engines
    EnginesListCommand,
    EnginesGetCommand,
    EnginesInitCommand,
    EnginesSetCommand,
  ],
})
export class CommandModule {}
