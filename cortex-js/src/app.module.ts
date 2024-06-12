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

@Module({
  imports: [
    DevtoolsModule.register({
      http: env.NODE_ENV !== 'production',
    }),
    ConfigModule.forRoot({
      isGlobal: true,
      envFilePath: env.NODE_ENV !== 'production' ? '.env.development' : '.env',
    }),
    DatabaseModule,
    MessagesModule,
    ThreadsModule,
    ModelsModule,
    ChatModule,
    AssistantsModule,
    CortexModule,
    ExtensionModule,
    FileManagerModule,
    ModelRepositoryModule,
  ],
  providers: [SeedService],
})
export class AppModule implements NestModule {
  configure(consumer: MiddlewareConsumer): void {
    consumer.apply(AppLoggerMiddleware).forRoutes('*');
  }
}
