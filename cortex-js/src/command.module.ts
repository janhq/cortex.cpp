import { Module } from '@nestjs/common';
import { ModelsModule } from './usecases/models/models.module';
import { DatabaseModule } from './infrastructure/database/database.module';
import { ConfigModule } from '@nestjs/config';
import { CortexModule } from './usecases/cortex/cortex.module';
import { ServeCommand } from './infrastructure/commanders/serve.command';
import { ModelsCommand } from './infrastructure/commanders/models.command';
import { ExtensionModule } from './infrastructure/repositories/extensions/extension.module';
import { InitCommand } from './infrastructure/commanders/init.command';
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
import { CliUsecasesModule } from './infrastructure/commanders/usecases/cli.usecases.module';
import { MessagesModule } from './usecases/messages/messages.module';
import { FileManagerModule } from './infrastructure/services/file-manager/file-manager.module';
import { PSCommand } from './infrastructure/commanders/ps.command';
import { KillCommand } from './infrastructure/commanders/kill.command';
import { PresetCommand } from './infrastructure/commanders/presets.command';
import { TelemetryModule } from './usecases/telemetry/telemetry.module';
import { TelemetryCommand } from './infrastructure/commanders/telemetry.command';
import { EmbeddingCommand } from './infrastructure/commanders/embeddings.command';
import { BenchmarkCommand } from './infrastructure/commanders/benchmark.command';
import { EventEmitterModule } from '@nestjs/event-emitter';
import { DownloadManagerModule } from './infrastructure/services/download-manager/download-manager.module';
import { ServeStopCommand } from './infrastructure/commanders/sub-commands/serve-stop.command';
import { ContextModule } from './infrastructure/services/context/context.module';
import { ExtensionsModule } from './extensions/extensions.module';
import { ConfigsCommand } from './infrastructure/commanders/configs.command';
import { EnginesCommand } from './infrastructure/commanders/engines.command';
import { ConfigsModule } from './usecases/configs/configs.module';
import { EnginesModule } from './usecases/engines/engines.module';
import { ConfigsGetCommand } from './infrastructure/commanders/configs/configs-get.command';
import { ConfigsListCommand } from './infrastructure/commanders/configs/configs-list.command';
import { ConfigsSetCommand } from './infrastructure/commanders/configs/configs-set.command';
import { EnginesListCommand } from './infrastructure/commanders/engines/engines-list.command';
import { EnginesGetCommand } from './infrastructure/commanders/engines/engines-get.command';

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
    ServeCommand,
    ChatCommand,
    InitCommand,
    PSCommand,
    KillCommand,
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

    // Configs
    ConfigsCommand,
    ConfigsGetCommand,
    ConfigsListCommand,
    ConfigsSetCommand,

    // Engines
    EnginesListCommand,
    EnginesGetCommand,
  ],
})
export class CommandModule {}
