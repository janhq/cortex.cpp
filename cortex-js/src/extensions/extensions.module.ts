import { Module } from '@nestjs/common';
import GroqEngineExtension from './groq.engine';
import MistralEngineExtension from './mistral.engine';
import OpenAIEngineExtension from './openai.engine';
import { HttpModule, HttpService } from '@nestjs/axios';
import { ConfigsUsecases } from '@/usecases/configs/configs.usecase';
import { FileManagerModule } from '@/infrastructure/services/file-manager/file-manager.module';

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
  // Do not import ConfigsModule here to avoid circular dependency
  imports: [HttpModule, FileManagerModule],
  controllers: [],
  providers: [ConfigsUsecases, provider],
  exports: [ConfigsUsecases, provider],
})
export class ExtensionsModule {}
