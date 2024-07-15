import { Inject, Injectable } from '@nestjs/common';
import { CreateMessageDto } from '@/infrastructure/dtos/messages/create-message.dto';
import { UpdateMessageDto } from '@/infrastructure/dtos/messages/update-message.dto';
import { ulid } from 'ulid';
import { MessageEntity } from '@/infrastructure/entities/message.entity';
import { Message } from '@/domain/models/message.interface';
import { Repository } from 'sequelize-typescript';

@Injectable()
export class MessagesUsecases {
  constructor(
    @Inject('MESSAGE_REPOSITORY')
    private messageRepository:  Repository<MessageEntity>,
  ) {}

  async create(createMessageDto: CreateMessageDto) {
    const { assistant_id } = createMessageDto;
    const message: Partial<Message> = {
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
    const messsage = await this.messageRepository.create(message)
    return messsage.toJSON();
  }

  async findAll() {
    const messages = await this.messageRepository.findAll();
    return messages.map((message) => message.toJSON());
  }

  async findOne(id: string) {
    const message = await this.messageRepository.findOne({
      where: {
        id,
      },
    });
    return message?.toJSON();
  }

  async update(id: string, updateMessageDto: UpdateMessageDto) {
    const [numberOfAffectedRows, [updatedMessage]] = await this.messageRepository.update(updateMessageDto, {
      where: { id },
      returning: true,
    });
    return { numberOfAffectedRows, updatedMessage: updatedMessage.toJSON() };
  }

  async remove(id: string) {
    return this.messageRepository.destroy({
      where: { id },
    });
  }

  async getLastMessagesByThread(threadId: string, limit: number) {
    const messages = await this.messageRepository.findAll({
      where: {
        thread_id: threadId,
      },
      order: [['created_at', 'DESC']],
      limit: limit,
    });
    return messages.map((message) => message.toJSON());
  }
}
