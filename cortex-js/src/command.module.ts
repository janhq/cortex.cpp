import { Module } from '@nestjs/common';
import { BasicCommand } from './infrastructure/commanders/basic-command.commander';
import { ModelsModule } from './usecases/models/models.module';
import { DatabaseModule } from './infrastructure/database/database.module';
import { ConfigModule } from '@nestjs/config';
import { CortexModule } from './usecases/cortex/cortex.module';
import { ServeCommand } from './infrastructure/commanders/serve.command';
import { ChatCommand } from './infrastructure/commanders/chat.command';
import { ModelsCommand } from './infrastructure/commanders/models.command';
import { StartCommand } from './infrastructure/commanders/start.command';
import { ExtensionModule } from './infrastructure/repositories/extensions/extension.module';
import { ChatModule } from './usecases/chat/chat.module';
import { InitCommand } from './infrastructure/commanders/init.command';
import { HttpModule } from '@nestjs/axios';
import { CreateInitQuestions } from './infrastructure/commanders/inquirer/init.questions';
import { ModelStartCommand } from './infrastructure/commanders/models/model-start.command';
import { ModelStopCommand } from './infrastructure/commanders/models/model-stop.command';
import { ModelListCommand } from './infrastructure/commanders/models/model-list.command';
import { ModelGetCommand } from './infrastructure/commanders/models/model-get.command';
import { ModelRemoveCommand } from './infrastructure/commanders/models/model-remove.command';
import { ModelPullCommand } from './infrastructure/commanders/models/model-pull.command';

@Module({
  imports: [
    ConfigModule.forRoot({
      isGlobal: true,
      envFilePath:
        process.env.NODE_ENV !== 'production' ? '.env.development' : '.env',
    }),
    DatabaseModule,
    ModelsModule,
    CortexModule,
    ChatModule,
    ExtensionModule,
    HttpModule,
  ],
  providers: [
    BasicCommand,
    ModelsCommand,
    ServeCommand,
    ChatCommand,
    StartCommand,
    InitCommand,
    CreateInitQuestions,

    // Model commands
    ModelStartCommand,
    ModelStopCommand,
    ModelListCommand,
    ModelGetCommand,
    ModelRemoveCommand,
    ModelPullCommand,
  ],
})
export class CommandModule {}
