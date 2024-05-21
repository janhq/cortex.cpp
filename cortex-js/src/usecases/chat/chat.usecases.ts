import { Inject, Injectable } from '@nestjs/common';
import { CreateChatCompletionDto } from '@/infrastructure/dtos/chat/create-chat-completion.dto';
import { ExtensionRepository } from '@/domain/repositories/extension.interface';
import { Repository } from 'typeorm';
import { ModelEntity } from '@/infrastructure/entities/model.entity';
import { EngineExtension } from '@/domain/abstracts/engine.abstract';
import stream from 'stream';
import { ModelNotFoundException } from '@/infrastructure/exception/model-not-found.exception';

@Injectable()
export class ChatUsecases {
  constructor(
    @Inject('MODEL_REPOSITORY')
    private readonly modelRepository: Repository<ModelEntity>,
    private readonly extensionRepository: ExtensionRepository,
  ) {}

  async inference(
    createChatDto: CreateChatCompletionDto,
    headers: Record<string, string>,
  ): Promise<any> {
    const { model: modelId } = createChatDto;
    const extensions = (await this.extensionRepository.findAll()) ?? [];
    const model = await this.modelRepository.findOne({
      where: { id: modelId },
    });

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

  async inferenceStream(
    createChatDto: CreateChatCompletionDto,
    headers: Record<string, string>,
  ): Promise<stream.Readable> {
    const { model: modelId } = createChatDto;
    const extensions = (await this.extensionRepository.findAll()) ?? [];
    const model = await this.modelRepository.findOne({
      where: { id: modelId },
    });

    if (!model) {
      throw new ModelNotFoundException(modelId);
    }

    const engine = extensions.find((e: any) => e.provider === model.engine) as
      | EngineExtension
      | undefined;
    if (engine == null) {
      throw new Error(`No engine found with name: ${model.engine}`);
    }

    return engine?.inferenceStream(createChatDto, headers);
  }
}
