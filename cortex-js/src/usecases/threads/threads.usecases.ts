import { Inject, Injectable, NotFoundException } from '@nestjs/common';
import { CreateThreadDto } from '@/infrastructure/dtos/threads/create-thread.dto';
import { UpdateThreadDto } from '@/infrastructure/dtos/threads/update-thread.dto';
import { ThreadEntity } from '@/infrastructure/entities/thread.entity';
import { Repository } from 'typeorm';
import { v4 as uuidv4 } from 'uuid';
import { MessageEntity } from '@/infrastructure/entities/message.entity';
import { PageDto } from '@/infrastructure/dtos/page.dto';
import { CreateMessageDto } from '@/infrastructure/dtos/threads/create-message.dto';
import { ulid } from 'ulid';
import { Message, MessageContent } from '@/domain/models/message.interface';
import { UpdateMessageDto } from '@/infrastructure/dtos/threads/update-message.dto';
import { Thread } from '@/domain/models/thread.interface';
import DeleteMessageDto from '@/infrastructure/dtos/threads/delete-message.dto';
import { AssistantEntity } from '@/infrastructure/entities/assistant.entity';

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
    const { assistants } = createThreadDto;
    const assistantEntity: AssistantEntity[] = assistants.map((assistant) => {
      const entity: AssistantEntity = {
        ...assistant,
        response_format: null,
        tool_resources: null,
        top_p: assistant.top_p ?? null,
        temperature: assistant.temperature ?? null,
      };
      return entity;
    });

    const thread: ThreadEntity = {
      id,
      assistants: assistantEntity,
      object: 'thread',
      created_at: Date.now(),
      title: 'New Thread',
      tool_resources: null,
      metadata: null,
    };
    await this.threadRepository.insert(thread);
    return thread;
  }

  async findAll(): Promise<ThreadEntity[]> {
    return this.threadRepository.find({
      order: {
        created_at: 'DESC',
      },
    });
  }

  async getMessagesOfThread(
    threadId: string,
    limit: number,
    order: 'asc' | 'desc',
    after?: string,
    before?: string,
    // eslint-disable-next-line @typescript-eslint/no-unused-vars
    runId?: string,
  ) {
    await this.getThreadOrThrow(threadId);
    const queryBuilder = this.messageRepository.createQueryBuilder();
    const normalizedOrder = order === 'asc' ? 'ASC' : 'DESC';

    queryBuilder
      .where('thread_id = :id', { id: threadId })
      .orderBy('created_at', normalizedOrder)
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

  async createMessageInThread(
    threadId: string,
    createMessageDto: CreateMessageDto,
  ) {
    const thread = await this.getThreadOrThrow(threadId);
    const assistantId: string = thread.assistants[0].id;

    const messageContent: MessageContent = {
      type: 'text',
      text: {
        annotations: [],
        value: createMessageDto.content,
      },
    };

    const message: MessageEntity = {
      id: ulid(),
      object: 'thread.message',
      thread_id: threadId,
      assistant_id: assistantId,
      created_at: Date.now(),
      status: 'completed',
      role: createMessageDto.role,
      content: [messageContent],
      metadata: null,
      run_id: null,
      completed_at: null,
      incomplete_details: null,
      attachments: [],
      incomplete_at: null,
    };
    await this.messageRepository.insert(message);
    return message;
  }

  async updateMessage(
    threadId: string,
    messageId: string,
    updateMessageDto: UpdateMessageDto,
  ) {
    await this.getThreadOrThrow(threadId);
    await this.messageRepository.update(messageId, updateMessageDto);
    return this.messageRepository.findOne({
      where: {
        id: messageId,
      },
    });
  }

  private async getThreadOrThrow(threadId: string): Promise<Thread> {
    const thread = await this.findOne(threadId);
    if (!thread) {
      throw new NotFoundException(`Thread with id ${threadId} not found`);
    }
    return thread;
  }

  private async getMessageOrThrow(messageId: string): Promise<Message> {
    const message = await this.messageRepository.findOne({
      where: {
        id: messageId,
      },
    });
    if (!message) {
      throw new NotFoundException(`Message with id ${messageId} not found`);
    }
    return message;
  }

  findOne(id: string) {
    return this.threadRepository.findOne({ where: { id } });
  }

  async update(id: string, updateThreadDto: UpdateThreadDto) {
    const assistantEntities: AssistantEntity[] =
      updateThreadDto.assistants?.map((assistant) => {
        const entity: AssistantEntity = {
          ...assistant,
          name: assistant.name,
          response_format: null,
          tool_resources: null,
          top_p: assistant.top_p ?? null,
          temperature: assistant.temperature ?? null,
        };
        return entity;
      }) ?? [];

    const entity: Partial<ThreadEntity> = {
      ...updateThreadDto,
      assistants: assistantEntities,
    };
    return this.threadRepository.update(id, entity);
  }

  remove(id: string) {
    this.threadRepository.delete(id);
  }

  async deleteMessage(
    _threadId: string,
    messageId: string,
  ): Promise<DeleteMessageDto> {
    // we still allow user to delete message even if the thread is not there
    await this.getMessageOrThrow(messageId);
    await this.messageRepository.delete(messageId);

    return {
      id: messageId,
      object: 'thread.message.deleted',
      deleted: true,
    };
  }

  async retrieveMessage(_threadId: string, messageId: string) {
    // we still allow user to delete message even if the thread is not there
    return this.getMessageOrThrow(messageId);
  }

  async clean(threadId: string) {
    await this.getThreadOrThrow(threadId);
    await this.messageRepository.delete({ thread_id: threadId });
  }
}
