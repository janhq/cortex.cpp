import { Injectable } from '@nestjs/common';
import { CreateChatCompletionDto } from '@/infrastructure/dtos/chat/create-chat-completion.dto';
import { EngineExtension } from '@/domain/abstracts/engine.abstract';
import { ModelNotFoundException } from '@/infrastructure/exception/model-not-found.exception';
import { ModelRepository } from '@/domain/repositories/model.interface';
import { ExtensionRepository } from '@/domain/repositories/extension.interface';

@Injectable()
export class ChatUsecases {
  constructor(
    private readonly modelRepository: ModelRepository,
    private readonly extensionRepository: ExtensionRepository,
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
}
