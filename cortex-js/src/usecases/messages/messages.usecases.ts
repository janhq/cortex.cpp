import { Inject, Injectable } from '@nestjs/common';
import { CreateMessageDto } from '@/infrastructure/dtos/messages/create-message.dto';
import { UpdateMessageDto } from '@/infrastructure/dtos/messages/update-message.dto';
import { MessageEntity } from '@/infrastructure/entities/message.entity';
import { ulid } from 'ulid';
import { Repository } from 'sequelize-typescript';
import { Message } from '@/domain/models/message.interface';

@Injectable()
export class MessagesUsecases {
  constructor(
    @Inject('MESSAGE_REPOSITORY')
    private messageRepository: any,
  ) {}

  async create(createMessageDto: CreateMessageDto) {
    const { assistant_id } = createMessageDto;
    const message: Message = {
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
    const updateEntity: Partial<Message> = {
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
