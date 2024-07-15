import { Inject, Injectable } from '@nestjs/common';
import { CreateAssistantDto } from '@/infrastructure/dtos/assistants/create-assistant.dto';
import { Assistant } from '@/domain/models/assistant.interface';
import { PageDto } from '@/infrastructure/dtos/page.dto';
import { ModelRepository } from '@/domain/repositories/model.interface';
import { ModelNotFoundException } from '@/infrastructure/exception/model-not-found.exception';
import { Op } from 'sequelize';
import { AssistantEntity } from '@/infrastructure/entities/assistant.entity';
import { Repository } from 'sequelize-typescript';

@Injectable()
export class AssistantsUsecases {
  constructor(
    @Inject('ASSISTANT_REPOSITORY')
    private readonly assistantRepository: Repository<AssistantEntity>,
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

    const assistant: Partial<Assistant> = {
      ...createAssistantDto,
      object: 'assistant',
      created_at: Date.now(),
      response_format: null,
      tool_resources: null,
      top_p: top_p ?? null,
      temperature: temperature ?? null,
    };

    try {
      await this.assistantRepository.create(assistant);
    } catch (err) {
      throw err;
    }

    const result = await this.findOne(id);
    return result?.toJSON();
  }

  async listAssistants(
    limit: number,
    order: 'asc' | 'desc',
    after?: string,
    before?: string,
  ) {
    const normalizedOrder = order === 'asc' ? 'ASC' : 'DESC';

    const where: any = {};
    if (after) {
      where.id = { [Op.gt]: after };
    }
    if (before) {
      where.id = { [Op.lt]: before };
    }

    const assistants = await this.assistantRepository.findAll({
      where,
      order: [['created_at', normalizedOrder]],
      limit: limit + 1,
    });

    let hasMore = false;
    if (assistants.length > limit) {
      hasMore = true;
      assistants.pop();
    }

    const firstId = assistants[0]?.id ?? undefined;
    const lastId = assistants[assistants.length - 1]?.id ?? undefined;

    return new PageDto(assistants.map(assistant => assistant.toJSON()), hasMore, firstId, lastId);
  }

  async findAll(): Promise<Assistant[]> {
    const assistants = await this.assistantRepository.findAll();
    return assistants.map((assistant) => assistant.toJSON());
  }

  async findOne(id: string) {
    const assistant = await this.assistantRepository.findOne({
      where: { id },
    });
    return assistant?.toJSON();
  }

  async remove(id: string) {
    return this.assistantRepository.destroy({
      where: { id },
    });
  }
}
