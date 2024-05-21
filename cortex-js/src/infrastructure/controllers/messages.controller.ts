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
import { ApiTags, ApiOperation } from '@nestjs/swagger';

@ApiTags('Messages')
@Controller('messages')
export class MessagesController {
  constructor(private readonly messagesService: MessagesUsecases) {}

  @ApiOperation({ summary: 'Create Message', description: "Creates a message in a thread." })
  @Post()
  create(@Body() createMessageDto: CreateMessageDto) {
    return this.messagesService.create(createMessageDto);
  }

  @ApiOperation({ summary: 'List Messages', description: "Retrieves all the messages in a thread." })
  @Get()
  findAll() {
    return this.messagesService.findAll();
  }

  @ApiOperation({ summary: 'Retrieve Message', description: "Retrieves a specific message defined by a message's `id` ." })
  @Get(':id')
  findOne(@Param('id') id: string) {
    return this.messagesService.findOne(id);
  }

  @ApiOperation({ summary: 'Update Message', description: "Updates a specific message defined by a message's `id` ." })
  @Patch(':id')
  update(@Param('id') id: string, @Body() updateMessageDto: UpdateMessageDto) {
    return this.messagesService.update(id, updateMessageDto);
  }

  @ApiOperation({ summary: 'Delete Message', description: "Deletes a specific message defined by a message's `id` ." })
  @Delete(':id')
  remove(@Param('id') id: string) {
    return this.messagesService.remove(id);
  }
}
