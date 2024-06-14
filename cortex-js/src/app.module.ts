import { MiddlewareConsumer, Module, NestModule } from '@nestjs/common';
import { MessagesModule } from './usecases/messages/messages.module';
import { ThreadsModule } from './usecases/threads/threads.module';
import { ModelsModule } from './usecases/models/models.module';
import { DevtoolsModule } from '@nestjs/devtools-integration';
import { DatabaseModule } from './infrastructure/database/database.module';
import { ChatModule } from './usecases/chat/chat.module';
import { AssistantsModule } from './usecases/assistants/assistants.module';
import { ExtensionModule } from './infrastructure/repositories/extensions/extension.module';
import { ModelRepositoryModule } from './infrastructure/repositories/models/model.module';
import { CortexModule } from './usecases/cortex/cortex.module';
import { ConfigModule } from '@nestjs/config';
import { env } from 'node:process';
import { SeedService } from './usecases/seed/seed.service';
import { FileManagerModule } from './infrastructure/services/file-manager/file-manager.module';
import { AppLoggerMiddleware } from './infrastructure/middlewares/app.logger.middleware';
import { TelemetryModule } from './usecases/telemetry/telemetry.module';
import { APP_FILTER } from '@nestjs/core';
import { GlobalExceptionFilter } from './infrastructure/exception/global.exception';
import { UtilModule } from './util/util.module';
import { EventEmitterModule } from '@nestjs/event-emitter';
import { AppController } from './infrastructure/controllers/app.controller';
import { DownloadManagerModule } from './download-manager/download-manager.module';

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
    UtilModule,
  ],
  providers: [
    SeedService,
    {
      provide: APP_FILTER,
      useClass: GlobalExceptionFilter,
    },
    ModelRepositoryModule,
    DownloadManagerModule,
    SeedService,
  ],
  controllers: [AppController],
})
export class AppModule implements NestModule {
  configure(consumer: MiddlewareConsumer): void {
    consumer.apply(AppLoggerMiddleware).forRoutes('*');
  }
}
