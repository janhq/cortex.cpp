import { Module } from '@nestjs/common';
import GroqEngineExtension from './groq.engine';
import MistralEngineExtension from './mistral.engine';
import OpenAIEngineExtension from './openai.engine';
import { HttpModule, HttpService } from '@nestjs/axios';
import { ConfigsUsecases } from '@/usecases/configs/configs.usecase';
import { ConfigsModule } from '@/usecases/configs/configs.module';

const provider = {
  provide: 'EXTENSIONS_PROVIDER',
  inject: [HttpService, ConfigsUsecases],
  useFactory: (httpService: HttpService, configUsecases: ConfigsUsecases) => [
    new OpenAIEngineExtension(httpService, configUsecases),
    new GroqEngineExtension(httpService, configUsecases),
    new MistralEngineExtension(httpService, configUsecases),
  ],
};

@Module({
  imports: [HttpModule, ConfigsModule],
  controllers: [],
  providers: [provider],
  exports: [provider],
})
export class ExtensionsModule {}
