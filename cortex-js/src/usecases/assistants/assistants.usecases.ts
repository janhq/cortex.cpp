import { Inject, Injectable } from '@nestjs/common';
import { AssistantEntity } from '@/infrastructure/entities/assistant.entity';
import { Repository } from 'typeorm';
import { CreateAssistantDto } from '@/infrastructure/dtos/assistants/create-assistant.dto';
import { Assistant } from '@/domain/models/assistant.interface';
import { PageDto } from '@/infrastructure/dtos/page.dto';

@Injectable()
export class AssistantsUsecases {
  constructor(
    @Inject('ASSISTANT_REPOSITORY')
    private assistantRepository: Repository<AssistantEntity>,
  ) {}

  async create(createAssistantDto: CreateAssistantDto) {
    const { top_p, temperature } = createAssistantDto;
    const assistant: AssistantEntity = {
      ...createAssistantDto,
      object: 'assistant',
      created_at: Date.now(),
      response_format: null,
      tool_resources: null,
      top_p: top_p ?? null,
      temperature: temperature ?? null,
    };
    await this.assistantRepository.insert(assistant);
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
