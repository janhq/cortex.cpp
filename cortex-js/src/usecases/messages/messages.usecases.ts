import { Inject, Injectable } from '@nestjs/common';
import { CreateMessageDto } from '@/infrastructure/dtos/messages/create-message.dto';
import { UpdateMessageDto } from '@/infrastructure/dtos/messages/update-message.dto';
import { ulid } from 'ulid';
import { MessageEntity } from '@/infrastructure/entities/message.entity';
import { Message } from '@/domain/models/message.interface';
import { InjectModel } from '@nestjs/sequelize';
import { Op } from 'sequelize';

@Injectable()
export class MessagesUsecases {
  constructor(
    @InjectModel(MessageEntity)
    private readonly messageModel: typeof MessageEntity,
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
    return this.messageModel.create(message);
  }

  async findAll() {
    return this.messageModel.findAll();
  }

  async findOne(id: string) {
    return this.messageModel.findOne({
      where: {
        id,
      },
    });
  }

  async update(id: string, updateMessageDto: UpdateMessageDto) {
    const [numberOfAffectedRows, [updatedMessage]] = await this.messageModel.update(updateMessageDto, {
      where: { id },
      returning: true,
    });
    return { numberOfAffectedRows, updatedMessage };
  }

  async remove(id: string) {
    return this.messageModel.destroy({
      where: { id },
    });
  }

  async getLastMessagesByThread(threadId: string, limit: number) {
    return this.messageModel.findAll({
      where: {
        thread_id: threadId,
      },
      order: [['created_at', 'DESC']],
      limit: limit,
    });
  }
}
