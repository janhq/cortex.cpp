import { Inject, Injectable } from '@nestjs/common';
import { CreateMessageDto } from './dto/create-message.dto';
import { UpdateMessageDto } from './dto/update-message.dto';
import { Repository } from 'typeorm';
import { MessageEntity } from './entities/message.entity';
import { ulid } from 'ulid';

@Injectable()
export class MessagesService {
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
