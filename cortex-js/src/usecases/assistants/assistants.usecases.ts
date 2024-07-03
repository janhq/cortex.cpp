import { Inject, Injectable } from '@nestjs/common';
import { CreateAssistantDto } from '@/infrastructure/dtos/assistants/create-assistant.dto';
import { Assistant } from '@/domain/models/assistant.interface';
import { PageDto } from '@/infrastructure/dtos/page.dto';
import { ModelRepository } from '@/domain/repositories/model.interface';
import { ModelNotFoundException } from '@/infrastructure/exception/model-not-found.exception';
import { DuplicateAssistantException } from '@/infrastructure/exception/duplicate-assistant.exception';
import { Repository } from 'sequelize-typescript';

@Injectable()
export class AssistantsUsecases {
  constructor(
    @Inject('ASSISTANT_REPOSITORY')
    private readonly assistantRepository: any,
    private readonly modelRepository: ModelRepository,
  ) {}

  async create(createAssistantDto: CreateAssistantDto) {
    const { top_p, temperature, model, id } = createAssistantDto;
    if (model !== '*') {
      const modelEntity = await this.modelRepository.findOne(model);
      if (!modelEntity) {
        throw new ModelNotFoundException(model);
      }
    }

    const assistant: Assistant = {
      ...createAssistantDto,
      object: 'assistant',
      created_at: Date.now(),
      response_format: null,
      tool_resources: null,
      top_p: top_p ?? null,
      temperature: temperature ?? null,
    };

    try {
      await this.assistantRepository.insert(assistant);
    } catch (err) {

      throw err;
    }

    return this.findOne(assistant.id);
  }

  async listAssistants(
    limit: number,
    order: 'asc' | 'desc',
    after?: string,
    before?: string,
  ) {
    const queryBuilder = this.assistantRepository.createQueryBuilder();
    const normalizedOrder = order === 'asc' ? 'ASC' : 'DESC';

    queryBuilder.orderBy('created_at', normalizedOrder).take(limit + 1);

    if (after) {
      queryBuilder.andWhere('id > :after', { after });
    }

    if (before) {
      queryBuilder.andWhere('id < :before', { before });
    }

    const { entities: assistants } = await queryBuilder.getRawAndEntities();

    let hasMore = false;
    if (assistants.length > limit) {
      hasMore = true;
      assistants.pop();
    }

    const firstId = assistants[0]?.id ?? undefined;
    const lastId = assistants[assistants.length - 1]?.id ?? undefined;

    return new PageDto(assistants, hasMore, firstId, lastId);
  }

  async findAll(): Promise<Assistant[]> {
    return this.assistantRepository.find();
  }

  async findOne(id: string) {
    return this.assistantRepository.findOne({
      where: { id },
    });
  }

  async remove(id: string) {
    return this.assistantRepository.delete(id);
  }
}
