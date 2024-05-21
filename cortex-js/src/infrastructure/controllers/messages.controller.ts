import {
  Controller,
  Get,
  Post,
  Body,
  Patch,
  Param,
  Delete,
  UseInterceptors,
} from '@nestjs/common';
import { MessagesUsecases } from '@/usecases/messages/messages.usecases';
import { CreateMessageDto } from '@/infrastructure/dtos/messages/create-message.dto';
import { UpdateMessageDto } from '@/infrastructure/dtos/messages/update-message.dto';
import { ApiTags } from '@nestjs/swagger';
import { TransformInterceptor } from '../interceptors/transform.interceptor';

@ApiTags('Messages')
@Controller('messages')
@UseInterceptors(TransformInterceptor)
export class MessagesController {
  constructor(private readonly messagesService: MessagesUsecases) {}

  @Post()
  create(@Body() createMessageDto: CreateMessageDto) {
    return this.messagesService.create(createMessageDto);
  }

  @Get()
  findAll() {
    return this.messagesService.findAll();
  }

  @Get(':id')
  findOne(@Param('id') id: string) {
    return this.messagesService.findOne(id);
  }

  @Patch(':id')
  update(@Param('id') id: string, @Body() updateMessageDto: UpdateMessageDto) {
    return this.messagesService.update(id, updateMessageDto);
  }

  @Delete(':id')
  remove(@Param('id') id: string) {
    return this.messagesService.remove(id);
  }
}
