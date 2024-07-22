import { Injectable } from '@nestjs/common';
import { CreateChatCompletionDto } from '@/infrastructure/dtos/chat/create-chat-completion.dto';
import { EngineExtension } from '@/domain/abstracts/engine.abstract';
import { ModelNotFoundException } from '@/infrastructure/exception/model-not-found.exception';
import { TelemetryUsecases } from '../telemetry/telemetry.usecases';
import { TelemetrySource } from '@/domain/telemetry/telemetry.interface';
import { ExtensionRepository } from '@/domain/repositories/extension.interface';
import { firstValueFrom } from 'rxjs';
import { HttpService } from '@nestjs/axios';
import {
  CORTEX_CPP_EMBEDDINGS_URL,
  defaultEmbeddingModel,
} from '@/infrastructure/constants/cortex';
import { CreateEmbeddingsDto } from '@/infrastructure/dtos/embeddings/embeddings-request.dto';
import { FileManagerService } from '@/infrastructure/services/file-manager/file-manager.service';
import { Engines } from '@/infrastructure/commanders/types/engine.interface';
import { ModelsUsecases } from '../models/models.usecases';

@Injectable()
export class ChatUsecases {
  constructor(
    private readonly extensionRepository: ExtensionRepository,
    private readonly telemetryUseCases: TelemetryUsecases,
    private readonly modelsUsescases: ModelsUsecases,
    private readonly httpService: HttpService,
    private readonly fileService: FileManagerService,
  ) {}

  async inference(
    createChatDto: CreateChatCompletionDto,
    headers: Record<string, string>,
  ): Promise<any> {
    const { model: modelId } = createChatDto;
    const model = await this.modelsUsescases.findOne(modelId);
    if (!model) {
      throw new ModelNotFoundException(modelId);
    }

    const isModelRunning = await this.modelsUsescases.isModelRunning(modelId);
    // If model is not running
    // Start the model
    if (!isModelRunning) {
      await this.modelsUsescases.startModel(modelId);
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
    const modelId = dto.model ?? defaultEmbeddingModel;

    if (modelId !== dto.model) dto = { ...dto, model: modelId };

    if (!(await this.modelsUsescases.findOne(modelId))) {
      await this.modelsUsescases.pullModel(modelId);
    }

    const isModelRunning = await this.modelsUsescases.isModelRunning(modelId);
    // If model is not running
    // Start the model
    if (!isModelRunning) {
      await this.modelsUsescases.startModel(modelId, {
        embedding: true,
        model_type: 'embedding',
      });
    }

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
