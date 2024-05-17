import { Inject, Injectable } from '@nestjs/common';
import { CreateChatCompletionDto } from '@/infrastructure/dtos/chat/create-chat-completion.dto';
import { ExtensionRepository } from '@/domain/repositories/extension.interface';
import { Repository } from 'typeorm';
import { ModelEntity } from '@/infrastructure/entities/model.entity';
import { EngineExtension } from '@/domain/abstracts/engine.abstract';
import { ChatStreamEvent } from '@/domain/abstracts/oai.abstract';

@Injectable()
export class ChatUsecases {
  constructor(
    @Inject('MODEL_REPOSITORY')
    private readonly modelRepository: Repository<ModelEntity>,
    private readonly extensionRepository: ExtensionRepository,
  ) {}

  async createChatCompletions(
    createChatDto: CreateChatCompletionDto,
    headers: Record<string, string>,
    stream: WritableStream<ChatStreamEvent>,
    res?: any,
  ) {
    const extensions = (await this.extensionRepository.findAll()) ?? [];
    const model = await this.modelRepository.findOne({
      where: { id: createChatDto.model },
    });
    const engine = extensions.find((e: any) => e.provider === model?.engine) as
      | EngineExtension
      | undefined;
    engine?.inference(createChatDto, headers, stream, res);
  }
}
