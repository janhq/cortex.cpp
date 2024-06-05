import { Inject, Injectable, NotFoundException } from '@nestjs/common';
import { CreateThreadDto } from '@/infrastructure/dtos/threads/create-thread.dto';
import { UpdateThreadDto } from '@/infrastructure/dtos/threads/update-thread.dto';
import { ThreadEntity } from '@/infrastructure/entities/thread.entity';
import { Repository } from 'typeorm';
import { v4 as uuidv4 } from 'uuid';
import { MessageEntity } from '@/infrastructure/entities/message.entity';
import { PageDto } from '@/infrastructure/dtos/page.dto';

@Injectable()
export class ThreadsUsecases {
  constructor(
    @Inject('THREAD_REPOSITORY')
    private threadRepository: Repository<ThreadEntity>,
    @Inject('MESSAGE_REPOSITORY')
    private messageRepository: Repository<MessageEntity>,
  ) {}

  async create(createThreadDto: CreateThreadDto): Promise<ThreadEntity> {
    const id = uuidv4();

    const thread: ThreadEntity = {
      ...createThreadDto,
      id,
      object: 'thread',
      createdAt: Date.now(),
    };
    await this.threadRepository.insert(thread);
    return thread;
  }

  async findAll(): Promise<ThreadEntity[]> {
    return this.threadRepository.find();
  }

  async getMessagesOfThread(
    id: string,
    limit: number,
    order: 'asc' | 'desc',
    after?: string,
    before?: string,
    runId?: string,
  ) {
    const thread = await this.findOne(id);
    if (!thread) {
      throw new NotFoundException(`Thread with id ${id} not found`);
    }

    const queryBuilder = this.messageRepository.createQueryBuilder();
    const normalizedOrder = order === 'asc' ? 'ASC' : 'DESC';

    queryBuilder
      .where('thread_id = :id', { id })
      .orderBy('created', normalizedOrder)
      .take(limit + 1); // Fetch one more record than the limit

    if (after) {
      queryBuilder.andWhere('id > :after', { after });
    }

    if (before) {
      queryBuilder.andWhere('id < :before', { before });
    }

    const { entities: messages } = await queryBuilder.getRawAndEntities();

    let hasMore = false;
    if (messages.length > limit) {
      hasMore = true;
      messages.pop(); // Remove the extra record
    }

    const firstId = messages[0]?.id ?? undefined;
    const lastId = messages[messages.length - 1]?.id ?? undefined;

    return new PageDto(messages, hasMore, firstId, lastId);
  }

  findOne(id: string) {
    return this.threadRepository.findOne({ where: { id } });
  }

  update(id: string, updateThreadDto: UpdateThreadDto) {
    return this.threadRepository.update(id, updateThreadDto);
  }

  remove(id: string) {
    this.threadRepository.delete(id);
  }
}
