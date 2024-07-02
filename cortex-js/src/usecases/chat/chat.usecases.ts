import { Injectable } from '@nestjs/common';
import { CreateChatCompletionDto } from '@/infrastructure/dtos/chat/create-chat-completion.dto';
import { EngineExtension } from '@/domain/abstracts/engine.abstract';
import { ModelNotFoundException } from '@/infrastructure/exception/model-not-found.exception';
import { TelemetryUsecases } from '../telemetry/telemetry.usecases';
import { TelemetrySource } from '@/domain/telemetry/telemetry.interface';
import { ModelRepository } from '@/domain/repositories/model.interface';
import { ExtensionRepository } from '@/domain/repositories/extension.interface';
import { firstValueFrom } from 'rxjs';
import { HttpService } from '@nestjs/axios';
import { CORTEX_CPP_EMBEDDINGS_URL } from '@/infrastructure/constants/cortex';
import { CreateEmbeddingsDto } from '@/infrastructure/dtos/embeddings/embeddings-request.dto';
import { FileManagerService } from '@/infrastructure/services/file-manager/file-manager.service';
import { Engines } from '@/infrastructure/commanders/types/engine.interface';

@Injectable()
export class ChatUsecases {
  constructor(
    private readonly modelRepository: ModelRepository,
    private readonly extensionRepository: ExtensionRepository,
    private readonly telemetryUseCases: TelemetryUsecases,
    private readonly httpService: HttpService,
    private readonly fileService: FileManagerService,
  ) {}

  async inference(
    createChatDto: CreateChatCompletionDto,
    headers: Record<string, string>,
  ): Promise<any> {
    const { model: modelId } = createChatDto;
    const model = await this.modelRepository.findOne(modelId);
    if (!model) {
      throw new ModelNotFoundException(modelId);
    }
    const engine = (await this.extensionRepository.findOne(
      model!.engine ?? Engines.llamaCPP,
    )) as EngineExtension | undefined;

    if (engine == null) {
      throw new Error(`No engine found with name: ${model.engine}`);
    }
    try {
      return await engine.inference(createChatDto, headers);
    } catch (error) {
      await this.telemetryUseCases.createCrashReport(
        error,
        TelemetrySource.CORTEX_CPP,
      );
      throw error;
    }
  }

  /**
   * Creates an embedding vector representing the input text.
   * @param model Embedding model ID.
   * @param input Input text to embed, encoded as a string or array of tokens. To embed multiple inputs in a single request, pass an array of strings or array of token arrays.
   * @param encoding_format Encoding format for the embeddings. Supported formats are 'float' and 'int'.
   * @param host Cortex CPP host.
   * @param port Cortex CPP port.
   * @returns Embedding vector.
   */
  async embeddings(dto: CreateEmbeddingsDto) {
    const configs = await this.fileService.getConfig();
    return firstValueFrom(
      this.httpService.post(
        CORTEX_CPP_EMBEDDINGS_URL(configs.cortexCppHost, configs.cortexCppPort),
        dto,
        {
          headers: {
            'Content-Type': 'application/json',
            'Accept-Encoding': 'gzip',
          },
        },
      ),
    ).then((res) => res.data);
  }
}
