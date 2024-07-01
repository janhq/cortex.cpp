import { Module } from '@nestjs/common';
import GroqEngineExtension from './groq.engine';
import MistralEngineExtension from './mistral.engine';
import OpenAIEngineExtension from './openai.engine';
import { HttpModule, HttpService } from '@nestjs/axios';
import { ConfigsUsecases } from '@/usecases/configs/configs.usecase';
import { ConfigsModule } from '@/usecases/configs/configs.module';
import { EventEmitter2, EventEmitterModule } from '@nestjs/event-emitter';

const provider = {
  provide: 'EXTENSIONS_PROVIDER',
  inject: [HttpService, ConfigsUsecases, EventEmitter2],
  useFactory: (
    httpService: HttpService,
    configUsecases: ConfigsUsecases,
    eventEmitter: EventEmitter2,
  ) => [
    new OpenAIEngineExtension(httpService, configUsecases, eventEmitter),
    new GroqEngineExtension(httpService, configUsecases, eventEmitter),
    new MistralEngineExtension(httpService, configUsecases, eventEmitter),
  ],
};

@Module({
  imports: [HttpModule, ConfigsModule],
  controllers: [],
  providers: [provider],
  exports: [provider],
})
export class ExtensionsModule {}
