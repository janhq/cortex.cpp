import { Injectable } from '@nestjs/common';
import { CreateChatCompletionDto } from '@/infrastructure/dtos/chat/create-chat-completion.dto';
import { EngineExtension } from '@/domain/abstracts/engine.abstract';
import { ModelNotFoundException } from '@/infrastructure/exception/model-not-found.exception';
import { ModelRepository } from '@/domain/repositories/model.interface';
import { ExtensionRepository } from '@/domain/repositories/extension.interface';
import { firstValueFrom } from 'rxjs';
import { HttpService } from '@nestjs/axios';
import { CORTEX_CPP_EMBEDDINGS_URL } from '@/infrastructure/constants/cortex';
import { CreateEmbeddingsDto } from '@/infrastructure/dtos/embeddings/embeddings-request.dto';

@Injectable()
export class ChatUsecases {
  constructor(
    private readonly modelRepository: ModelRepository,
    private readonly extensionRepository: ExtensionRepository,
    private readonly httpService: HttpService,
  ) {}

  async inference(
    createChatDto: CreateChatCompletionDto,
    headers: Record<string, string>,
  ): Promise<any> {
    const { model: modelId } = createChatDto;
    const extensions = (await this.extensionRepository.findAll()) ?? [];
    const model = await this.modelRepository.findOne(modelId);

    if (!model) {
      throw new ModelNotFoundException(modelId);
    }
    const engine = extensions.find((e: any) => e.provider === model?.engine) as
      | EngineExtension
      | undefined;

    if (engine == null) {
      throw new Error(`No engine found with name: ${model.engine}`);
    }
    return engine.inference(createChatDto, headers);
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
  embeddings(dto: CreateEmbeddingsDto) {
    return firstValueFrom(
      this.httpService.post(CORTEX_CPP_EMBEDDINGS_URL(), dto, {
        headers: {
          'Content-Type': 'application/json',
          'Accept-Encoding': 'gzip',
        },
      }),
    ).then((res) => res.data);
  }
}
