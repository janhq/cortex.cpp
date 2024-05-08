import { Module } from '@nestjs/common';
import { BasicCommand } from './infrastructure/commanders/basic-command.commander';
import { ModelsModule } from './usecases/models/models.module';
import { DatabaseModule } from './infrastructure/database/database.module';
import { ConfigModule } from '@nestjs/config';
import { CortexModule } from './usecases/cortex/cortex.module';

@Module({
  imports: [
    DatabaseModule,
    ModelsModule,
    CortexModule,
    ConfigModule.forRoot({
      isGlobal: true,
      envFilePath:
        process.env.NODE_ENV === 'production' ? '.env' : '.env.development',
    }),
  ],
  providers: [BasicCommand],
})
export class CommandModule {}
