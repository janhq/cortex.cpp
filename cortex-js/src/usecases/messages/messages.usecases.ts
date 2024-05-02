import { Inject, Injectable } from '@nestjs/common';
import { CreateMessageDto } from '../../infrastructure/dtos/messages/create-message.dto';
import { UpdateMessageDto } from '../../infrastructure/dtos/messages/update-message.dto';
import { Repository } from 'typeorm';
import { MessageEntity } from '../../infrastructure/entities/message.entity';
import { ulid } from 'ulid';

@Injectable()
export class MessagesUsecases {
  constructor(
    @Inject('MESSAGE_REPOSITORY')
    private messageRepository: Repository<MessageEntity>,
  ) {}

  create(createMessageDto: CreateMessageDto) {
    const message: MessageEntity = {
      ...createMessageDto,
      id: ulid(),
      object: 'message',
      created: Date.now(),
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
    return this.messageRepository.update(id, updateMessageDto);
  }

  remove(id: string) {
    return this.messageRepository.delete(id);
  }
}
