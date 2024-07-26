import { Module } from '@nestjs/common';
import { ConfigModule } from '@nestjs/config';
import { CortexModule } from './usecases/cortex/cortex.module';
import { ModelsCommand } from './infrastructure/commanders/models.command';
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
import { RunCommand } from './infrastructure/commanders/run.command';
import { ModelUpdateCommand } from './infrastructure/commanders/models/model-update.command';
import { FileManagerModule } from './infrastructure/services/file-manager/file-manager.module';
import { PSCommand } from './infrastructure/commanders/ps.command';
import { PresetCommand } from './infrastructure/commanders/presets.command';
import { TelemetryModule } from './usecases/telemetry/telemetry.module';
import { TelemetryCommand } from './infrastructure/commanders/telemetry.command';
import { EmbeddingCommand } from './infrastructure/commanders/embeddings.command';
import { BenchmarkCommand } from './infrastructure/commanders/benchmark.command';
import { ServeStopCommand } from './infrastructure/commanders/serve-stop.command';
import { ContextModule } from './infrastructure/services/context/context.module';
import { EnginesCommand } from './infrastructure/commanders/engines.command';
import { EnginesListCommand } from './infrastructure/commanders/engines/engines-list.command';
import { EnginesGetCommand } from './infrastructure/commanders/engines/engines-get.command';
import { EnginesInitCommand } from './infrastructure/commanders/engines/engines-init.command';
import { EnginesSetCommand } from './infrastructure/commanders/engines/engines-set.command';
import { CortexClientModule } from './infrastructure/commanders/services/cortex.client.module';

@Module({
  imports: [
    ConfigModule.forRoot({
      isGlobal: true,
      envFilePath:
        process.env.NODE_ENV !== 'production' ? '.env.development' : '.env',
    }),
    CortexModule,
    HttpModule,
    FileManagerModule,
    TelemetryModule,
    ContextModule,
    CortexClientModule,
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
