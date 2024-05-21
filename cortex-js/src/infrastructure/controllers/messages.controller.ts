import {
  Controller,
  Get,
  Post,
  Body,
  Patch,
  Param,
  HttpCode,
  Delete,
} from '@nestjs/common';
import { MessagesUsecases } from '@/usecases/messages/messages.usecases';
import { CreateMessageDto } from '@/infrastructure/dtos/messages/create-message.dto';
import { UpdateMessageDto } from '@/infrastructure/dtos/messages/update-message.dto';
import {
  ApiCreatedResponse,
  ApiOkResponse,
  ApiOperation,
  ApiParam,
  ApiTags,
  ApiResponse
} from '@nestjs/swagger';

@ApiTags('Messages')
@Controller('messages')
export class MessagesController {
  constructor(private readonly messagesService: MessagesUsecases) {}

  @HttpCode(200)
  @ApiResponse({
    status: 200,
    description: 'Successfully created the message.',
    type: CreateMessageDto,
  })
  @ApiOperation({ summary: 'Create message', description: "Creates a message in a thread." })
  @Post()
  create(@Body() createMessageDto: CreateMessageDto) {
    return this.messagesService.create(createMessageDto);
  }

  @ApiOperation({ summary: 'List messages', description: "Retrieves all the messages in a thread." })
  @Get()
  findAll() {
    return this.messagesService.findAll();
  }

  @ApiOperation({ summary: 'Retrieve message', description: "Retrieves a specific message defined by a message's `id`." })
  @ApiParam({ name: 'id', required: true, description: "The unique identifier of the message." })
  @Get(':id')
  findOne(@Param('id') id: string) {
    return this.messagesService.findOne(id);
  }

  @HttpCode(200)
  @ApiResponse({
    status: 200,
    description: 'Successfully updated the message.',
    type: UpdateMessageDto,
  })
  @ApiOperation({ summary: 'Update message', description: "Updates a specific message defined by a message's `id`." })
  @ApiParam({ name: 'id', required: true, description: "The unique identifier of the message." })
  @Patch(':id')
  update(@Param('id') id: string, @Body() updateMessageDto: UpdateMessageDto) {
    return this.messagesService.update(id, updateMessageDto);
  }

  @HttpCode(200)
  @ApiResponse({
    status: 200,
    description: 'Successfully deleted the message.'
  })
  @ApiOperation({ summary: 'Delete message', description: "Deletes a specific message defined by a message's `id`." })
  @ApiParam({ name: 'id', required: true, description: "The unique identifier of the message." })
  @Delete(':id')
  remove(@Param('id') id: string) {
    return this.messagesService.remove(id);
  }
}
