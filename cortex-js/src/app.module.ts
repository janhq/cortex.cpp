import { Module } from '@nestjs/common';
import { MessagesModule } from './usecases/messages/messages.module';
import { ThreadsModule } from './usecases/threads/threads.module';
import { ModelsModule } from './usecases/models/models.module';
import { DevtoolsModule } from '@nestjs/devtools-integration';
import { DatabaseModule } from './infrastructure/database/database.module';
import { ChatModule } from './usecases/chat/chat.module';
import { AssistantsModule } from './usecases/assistants/assistants.module';
import { InferenceSettingsModule } from './usecases/inference-settings/inference-settings.module';
import { ExtensionModule } from './infrastructure/repositories/extensions/extension.module';
import { CortexModule } from './usecases/cortex/cortex.module';
import { ConfigModule } from '@nestjs/config';

@Module({
  imports: [
    DevtoolsModule.register({
      http: process.env.NODE_ENV !== 'production',
    }),
    ConfigModule.forRoot({
      isGlobal: true,
      envFilePath:
        process.env.NODE_ENV === 'production' ? '.env' : '.env.development',
    }),
    DatabaseModule,
    MessagesModule,
    ThreadsModule,
    ModelsModule,
    CoreModule,
    ChatModule,
    AssistantsModule,
    InferenceSettingsModule,
    CortexModule,
    UsersModule,
    ExtensionModule,
  ],
})
export class AppModule {}
