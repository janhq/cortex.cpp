console.time('import-command');
console.time('import-command-module');
import { Module } from '@nestjs/common';
console.timeEnd('import-command-module');
console.time('import-command-model-module');
import { ModelsModule } from './usecases/models/models.module';
console.timeEnd('import-command-model-module');
console.time('import-command-database-module');
import { DatabaseModule } from './infrastructure/database/database.module';
console.timeEnd('import-command-database-module');
console.time('import-command-config-module');
import { ConfigModule } from '@nestjs/config';
console.timeEnd('import-command-config-module');
console.time('import-command-cortex-module');
import { CortexModule } from './usecases/cortex/cortex.module';
console.timeEnd('import-command-cortex-module');
console.time('import-command-server-command');
import { ServeCommand } from './infrastructure/commanders/serve.command';
console.timeEnd('import-command-server-command');
console.time('import-command-models-command');
import { ModelsCommand } from './infrastructure/commanders/models.command';
console.timeEnd('import-command-models-command');
console.time('import-command-extension-module');
import { ExtensionModule } from './infrastructure/repositories/extensions/extension.module';
console.timeEnd('import-command-extension-module');
console.time('import-command-init-command');
import { InitCommand } from './infrastructure/commanders/init.command';
console.timeEnd('import-command-init-command');
console.time('import-command-http-module');
import { HttpModule } from '@nestjs/axios';
console.timeEnd('import-command-http-module');
console.time('import-command-questions');
import { InitRunModeQuestions } from './infrastructure/commanders/questions/init.questions';
console.timeEnd('import-command-questions');
console.time('import-command-model-list-command');
import { ModelListCommand } from './infrastructure/commanders/models/model-list.command';
console.timeEnd('import-command-model-list-command');
console.time('import-command-model-pull-command');
import { ModelPullCommand } from './infrastructure/commanders/models/model-pull.command';
console.timeEnd('import-command-model-pull-command');
console.time('import-command-cortex-command');
import { CortexCommand } from './infrastructure/commanders/cortex-command.commander';
console.timeEnd('import-command-cortex-command');
console.time('import-command-chat-command');
import { ChatCommand } from './infrastructure/commanders/chat.command';
console.timeEnd('import-command-chat-command');
console.time('import-command-model-start-command');
import { ModelStartCommand } from './infrastructure/commanders/models/model-start.command';
console.timeEnd('import-command-model-start-command');
console.time('import-command-model-stop-command');
import { ModelStopCommand } from './infrastructure/commanders/models/model-stop.command';
console.timeEnd('import-command-model-stop-command');
console.time('import-command-model-get-command');
import { ModelGetCommand } from './infrastructure/commanders/models/model-get.command';
console.timeEnd('import-command-model-get-command');
console.time('import-command-model-remove-command');
import { ModelRemoveCommand } from './infrastructure/commanders/models/model-remove.command';
console.timeEnd('import-command-model-remove-command');
console.time('import-command-model-update-command');
import { RunCommand } from './infrastructure/commanders/shortcuts/run.command';
console.timeEnd('import-command-model-update-command');
console.time('import-command-model-update-command');
import { ModelUpdateCommand } from './infrastructure/commanders/models/model-update.command';
console.timeEnd('import-command-model-update-command');
console.time('import-command-assistants-module');
import { AssistantsModule } from './usecases/assistants/assistants.module';
console.timeEnd('import-command-assistants-module');
console.time('import-command-messages-module');
import { MessagesModule } from './usecases/messages/messages.module';
console.timeEnd('import-command-messages-module');
console.time('import-command-file-manager-module');
import { FileManagerModule } from './infrastructure/services/file-manager/file-manager.module';
console.timeEnd('import-command-file-manager-module');
console.time('import-command-ps-command');
import { PSCommand } from './infrastructure/commanders/ps.command';
console.timeEnd('import-command-ps-command');
console.time('import-command-kill-command');
import { KillCommand } from './infrastructure/commanders/kill.command';
console.timeEnd('import-command-kill-command');
console.time('import-command-preset-command');
import { PresetCommand } from './infrastructure/commanders/presets.command';
console.timeEnd('import-command-preset-command');
console.time('import-command-telemetry-module');
import { TelemetryModule } from './usecases/telemetry/telemetry.module';
console.timeEnd('import-command-telemetry-module');
console.time('import-command-telemetry-command');
import { TelemetryCommand } from './infrastructure/commanders/telemetry.command';
console.timeEnd('import-command-telemetry-command');
console.time('import-command-embedding-command');
import { EmbeddingCommand } from './infrastructure/commanders/embeddings.command';
console.timeEnd('import-command-embedding-command');
console.time('import-command-benchmark-command');
import { BenchmarkCommand } from './infrastructure/commanders/benchmark.command';
console.timeEnd('import-command-benchmark-command');
console.time('import-command-event-emitter');
import { EventEmitterModule } from '@nestjs/event-emitter';
console.timeEnd('import-command-event-emitter');
console.time('import-command-download-manager');
import { DownloadManagerModule } from './infrastructure/services/download-manager/download-manager.module';
console.timeEnd('import-command-download-manager');
console.time('import-command-servestop-command');
import { ServeStopCommand } from './infrastructure/commanders/sub-commands/serve-stop.command';
console.timeEnd('import-command-servestop-command');
console.time('import-command-context-module');
import { ContextModule } from './infrastructure/services/context/context.module';
console.timeEnd('import-command-context-module');
console.time('import-command-cli-usecases-module');
import { CliUsecasesModule } from './infrastructure/commanders/usecases/cli.usecases.module';
console.timeEnd('import-command-cli-usecases-module');
console.time('import-command-extensions-module');
import { ExtensionsModule } from './extensions/extensions.module';
console.timeEnd('import-command-extensions-module');
console.time('import-command-configs-command');
import { ConfigsCommand } from './infrastructure/commanders/configs.command';
console.timeEnd('import-command-configs-command');
console.time('import-command-engines-command');
import { EnginesCommand } from './infrastructure/commanders/engines.command';
console.timeEnd('import-command-engines-command');
console.time('import-command-configs-module');
import { ConfigsModule } from './usecases/configs/configs.module';
console.timeEnd('import-command-configs-module');
console.time('import-command-engines-module');
import { EnginesModule } from './usecases/engines/engines.module';
console.timeEnd('import-command-engines-module');
console.time('import-command-configs-get-command');
import { ConfigsGetCommand } from './infrastructure/commanders/configs/configs-get.command';
console.timeEnd('import-command-configs-get-command');
console.time('import-command-configs-list-command');
import { ConfigsListCommand } from './infrastructure/commanders/configs/configs-list.command';
console.timeEnd('import-command-configs-list-command');
console.time('import-command-configs-set-command');
import { ConfigsSetCommand } from './infrastructure/commanders/configs/configs-set.command';
console.timeEnd('import-command-configs-set-command');
console.time('import-command-engines-list-command');
import { EnginesListCommand } from './infrastructure/commanders/engines/engines-list.command';
console.timeEnd('import-command-engines-list-command');
console.time('import-command-engines-get-command');
import { EnginesGetCommand } from './infrastructure/commanders/engines/engines-get.command';
console.timeEnd('import-command-engines-get-command');
console.timeEnd('import-command');

console.time('import-command');

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

    // // Configs
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
