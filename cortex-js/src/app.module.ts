import { MiddlewareConsumer, Module, NestModule } from '@nestjs/common';
import { MessagesModule } from './usecases/messages/messages.module';
import { ThreadsModule } from './usecases/threads/threads.module';
import { ModelsModule } from './usecases/models/models.module';
import { DevtoolsModule } from '@nestjs/devtools-integration';
import { DatabaseModule } from './infrastructure/database/database.module';
import { ChatModule } from './usecases/chat/chat.module';
import { AssistantsModule } from './usecases/assistants/assistants.module';
import { ExtensionModule } from './infrastructure/repositories/extensions/extension.module';
import { CortexModule } from './usecases/cortex/cortex.module';
import { ConfigModule } from '@nestjs/config';
import { env } from 'node:process';
import { FileManagerModule } from './infrastructure/services/file-manager/file-manager.module';
import { AppLoggerMiddleware } from './infrastructure/middlewares/app.logger.middleware';
import { TelemetryModule } from './usecases/telemetry/telemetry.module';
import { APP_FILTER } from '@nestjs/core';
import { GlobalExceptionFilter } from './infrastructure/exception/global.exception';
import { EventEmitterModule } from '@nestjs/event-emitter';
import { EventsController } from './infrastructure/controllers/events.controller';
import { AssistantsController } from './infrastructure/controllers/assistants.controller';
import { ChatController } from './infrastructure/controllers/chat.controller';
import { EmbeddingsController } from './infrastructure/controllers/embeddings.controller';
import { ModelsController } from './infrastructure/controllers/models.controller';
import { ThreadsController } from './infrastructure/controllers/threads.controller';
import { StatusController } from './infrastructure/controllers/status.controller';
import { ProcessController } from './infrastructure/controllers/process.controller';
import { DownloadManagerModule } from './infrastructure/services/download-manager/download-manager.module';
import { ContextModule } from './infrastructure/services/context/context.module';
import { ExtensionsModule } from './extensions/extensions.module';
import { ConfigsModule } from './usecases/configs/configs.module';
import { EnginesModule } from './usecases/engines/engines.module';
import { ConfigsController } from './infrastructure/controllers/configs.controller';
import { EnginesController } from './infrastructure/controllers/engines.controller';
import { ResourceManagerModule } from './infrastructure/services/resources-manager/resources-manager.module';

@Module({
  imports: [
    DevtoolsModule.register({
      http: env.NODE_ENV !== 'production',
    }),
    ConfigModule.forRoot({
      isGlobal: true,
      envFilePath: env.NODE_ENV !== 'production' ? '.env.development' : '.env',
    }),
    EventEmitterModule.forRoot(),
    DownloadManagerModule,
    DatabaseModule,
    MessagesModule,
    ThreadsModule,
    ModelsModule,
    ChatModule,
    AssistantsModule,
    CortexModule,
    ExtensionModule,
    FileManagerModule,
    TelemetryModule,
    ContextModule,
    DownloadManagerModule,
    ExtensionsModule,
    ConfigsModule,
    EnginesModule,
    ResourceManagerModule,
  ],
  controllers: [
    AssistantsController,
    ChatController,
    EmbeddingsController,
    ModelsController,
    ThreadsController,
    StatusController,
    ProcessController,
    EventsController,
    ConfigsController,
    EnginesController,
  ],
  providers: [
    {
      provide: APP_FILTER,
      useClass: GlobalExceptionFilter,
    },
  ],
})
export class AppModule implements NestModule {
  configure(consumer: MiddlewareConsumer): void {
    consumer.apply(AppLoggerMiddleware).forRoutes('*');
  }
}
