import { Module } from '@nestjs/common';
import { AppController } from './app.controller';
import { ThreadsController } from './threads/threads.controller';
import { ModelsController } from './models/models.controller';
import { MessagesModule } from './messages/messages.module';
import { ThreadsModule } from './threads/threads.module';
import { ModelsModule } from './models/models.module';
import { DevtoolsModule } from '@nestjs/devtools-integration';
import { CoreModule } from './core/core.module';
import { DatabaseModule } from './database/database.module';
import { ChatModule } from './chat/chat.module';
import { AssistantsModule } from './assistants/assistants.module';

@Module({
  imports: [
    DevtoolsModule.register({
      http: process.env.NODE_ENV !== 'production',
    }),
    DatabaseModule,
    MessagesModule,
    ThreadsModule,
    ModelsModule,
    CoreModule,
    ChatModule,
    AssistantsModule,
  ],
  controllers: [AppController, ThreadsController, ModelsController],
})
export class AppModule {}
