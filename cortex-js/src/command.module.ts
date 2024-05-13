import { Module } from '@nestjs/common';
import { BasicCommand } from './infrastructure/commanders/basic-command.commander';
import { ModelsModule } from './usecases/models/models.module';
import { DatabaseModule } from './infrastructure/database/database.module';
import { ConfigModule } from '@nestjs/config';
import { CortexModule } from './usecases/cortex/cortex.module';
import { ServeCommand } from './infrastructure/commanders/serve.command';
import { PullCommand } from './infrastructure/commanders/pull.command';
import { InferenceCommand } from './infrastructure/commanders/inference.command';
import { ModelsCommand } from './infrastructure/commanders/models.command';
import { StartCommand } from './infrastructure/commanders/start.command';

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
  ],
  providers: [
    BasicCommand,
    ModelsCommand,
    PullCommand,
    ServeCommand,
    InferenceCommand,
    StartCommand,
  ],
})
export class CommandModule {}
