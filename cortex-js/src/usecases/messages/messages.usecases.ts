import { Inject, Injectable } from '@nestjs/common';
import { CreateMessageDto } from '@/infrastructure/dtos/messages/create-message.dto';
import { UpdateMessageDto } from '@/infrastructure/dtos/messages/update-message.dto';
import { Repository } from 'typeorm';
import { MessageEntity } from '@/infrastructure/entities/message.entity';
import { ulid } from 'ulid';

@Injectable()
export class MessagesUsecases {
  constructor(
    @Inject('MESSAGE_REPOSITORY')
    private messageRepository: Repository<MessageEntity>,
  ) {}

  async create(createMessageDto: CreateMessageDto) {
    const { assistant_id } = createMessageDto;
    const message: MessageEntity = {
      ...createMessageDto,
      id: ulid(),
      created_at: Date.now(),
      object: 'thread.message',
      run_id: null,
      completed_at: null,
      incomplete_details: null,
      attachments: [],
      incomplete_at: null,
      metadata: undefined,
      assistant_id: assistant_id ?? null,
    };
    this.messageRepository.insert(message);
  }

  findAll() {
    return this.messageRepository.find();
  }

  findOne(id: string) {
    return this.messageRepository.findOne({
      where: {
        id,
      },
    });
  }

  update(id: string, updateMessageDto: UpdateMessageDto) {
    const updateEntity: Partial<MessageEntity> = {
      ...updateMessageDto,
    };
    return this.messageRepository.update(id, updateEntity);
  }

  remove(id: string) {
    return this.messageRepository.delete(id);
  }

  async getLastMessagesByThread(threadId: string, limit: number) {
    return this.messageRepository.find({
      where: {
        thread_id: threadId,
      },
      order: {
        created_at: 'DESC',
      },
      take: limit,
    });
  }
}
