import { PartialType } from '@nestjs/mapped-types';
import { MessageEntity } from '@/infrastructure/entities/message.entity';

export class UpdateMessageDto extends PartialType(MessageEntity) {}
