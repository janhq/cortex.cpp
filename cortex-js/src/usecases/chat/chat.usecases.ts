import { Inject, Injectable } from '@nestjs/common';
import { CreateChatCompletionDto } from '@/infrastructure/dtos/chat/create-chat-completion.dto';
import { ExtensionRepository } from '@/domain/repositories/extension.interface';
import { Repository } from 'typeorm';
import { ModelEntity } from '@/infrastructure/entities/model.entity';
import { EngineExtension } from '@/domain/abstracts/engine.abstract';
import { ModelNotFoundException } from '@/infrastructure/exception/model-not-found.exception';
import { TelemetryUsecases } from '../telemetry/telemetry.usecases';
import { TelemetrySource } from '@/domain/telemetry/telemetry.interface';

@Injectable()
export class ChatUsecases {
  constructor(
    @Inject('MODEL_REPOSITORY')
    private readonly modelRepository: Repository<ModelEntity>,
    private readonly extensionRepository: ExtensionRepository,
    private readonly telemetryUseCases: TelemetryUsecases,
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
}
