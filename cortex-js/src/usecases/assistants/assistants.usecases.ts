import { BadRequestException, Inject, Injectable } from '@nestjs/common';
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

  create(createAssistantDto: CreateAssistantDto) {
    const assistant: Assistant = {
      ...createAssistantDto,
      object: 'assistant',
      created_at: Date.now(),
    };
    this.assistantRepository.insert(assistant);
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

    if (assistants.length === 0) {
      assistants.push(this.janAssistant);
    }

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
    if (id === this.janAssistant.id) {
      return this.janAssistant;
    }

    return this.assistantRepository.findOne({
      where: { id },
    });
  }

  async remove(id: string) {
    if (id === this.janAssistant.id) {
      throw new BadRequestException('Cannot delete Jan assistant!');
    }
    return this.assistantRepository.delete(id);
  }

  janAssistant: Assistant = {
    avatar: '',
    id: 'jan',
    object: 'assistant',
    created_at: Date.now(),
    name: 'Jan',
    description: 'A default assistant that can use all downloaded models',
    model: '*',
    instructions: '',
    tools: [
      {
        type: 'retrieval',
        enabled: false,
        settings: {
          top_k: 2,
          chunk_size: 1024,
          chunk_overlap: 64,
          retrieval_template:
            "Use the following pieces of context to answer the question at the end. If you don't know the answer, just say that you don't know, don't try to make up an answer.\n----------------\nCONTEXT: {CONTEXT}\n----------------\nQUESTION: {QUESTION}\n----------------\nHelpful Answer:",
        },
      },
    ],
    file_ids: [],
  };

  async seed() {
    if ((await this.findOne(this.janAssistant.id)) != null) {
      return;
    }
    await this.assistantRepository.insert(this.janAssistant);
  }
}
