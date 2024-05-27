import {
  Controller,
  Get,
  Post,
  Body,
  Patch,
  Param,
  Delete,
} from '@nestjs/common';
import { MessagesUsecases } from '@/usecases/messages/messages.usecases';
import { CreateMessageDto } from '@/infrastructure/dtos/messages/create-message.dto';
import { UpdateMessageDto } from '@/infrastructure/dtos/messages/update-message.dto';
import { ApiTags } from '@nestjs/swagger';

@ApiTags('Messages')
@Controller('messages')
export class MessagesController {
  constructor(private readonly messagesUsecases: MessagesUsecases) {}

  @Post()
  create(@Body() createMessageDto: CreateMessageDto) {
    return this.messagesUsecases.create(createMessageDto);
  }

  @Get()
  findAll() {
    return this.messagesUsecases.findAll();
  }

  @Get(':id')
  findOne(@Param('id') id: string) {
    return this.messagesUsecases.findOne(id);
  }

  @Patch(':id')
  update(@Param('id') id: string, @Body() updateMessageDto: UpdateMessageDto) {
    return this.messagesUsecases.update(id, updateMessageDto);
  }

  @Delete(':id')
  remove(@Param('id') id: string) {
    return this.messagesUsecases.remove(id);
  }
}
