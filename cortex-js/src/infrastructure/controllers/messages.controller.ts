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
import { ListMessagesResponseDto } from '@/infrastructure/dtos/messages/list-message.dto';
import { GetMessageResponseDto } from '@/infrastructure/dtos/messages/get-message.dto';
import { DeleteMessageResponseDto } from '@/infrastructure/dtos/messages/delete-message.dto';
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
  constructor(private readonly messagesUsecases: MessagesUsecases) {}

  @HttpCode(201)
  @ApiResponse({
    status: 201,
    description: 'The message has been successfully created.',
    type: CreateMessageDto,
  })
  @ApiOperation({ summary: 'Create message', description: "Creates a message in a thread." })
  @Post()
  create(@Body() createMessageDto: CreateMessageDto) {
    return this.messagesUsecases.create(createMessageDto);
  }

  @HttpCode(200)
  @ApiResponse({
    status: 200,
    description: 'Ok',
    type: ListMessagesResponseDto,
  })
  @ApiOperation({ summary: 'List messages', description: "Retrieves all the messages in a thread." })
  @Get()
  findAll() {
    return this.messagesUsecases.findAll();
  }

  @HttpCode(200)
  @ApiResponse({
    status: 200,
    description: 'Ok',
    type: GetMessageResponseDto,
  })
  @ApiOperation({ summary: 'Retrieve message', description: "Retrieves a specific message defined by a message's `id`." })
  @ApiParam({ name: 'id', required: true, description: "The unique identifier of the message." })
  @Get(':id')
  findOne(@Param('id') id: string) {
    return this.messagesUsecases.findOne(id);
  }

  @HttpCode(200)
  @ApiResponse({
    status: 200,
    description: 'The message has been successfully updated.',
    type: UpdateMessageDto,
  })
  @ApiOperation({ summary: 'Update message', description: "Updates a specific message defined by a message's `id`." })
  @ApiParam({ name: 'id', required: true, description: "The unique identifier of the message." })
  @Patch(':id')
  update(@Param('id') id: string, @Body() updateMessageDto: UpdateMessageDto) {
    return this.messagesUsecases.update(id, updateMessageDto);
  }

  @HttpCode(200)
  @ApiResponse({
    status: 200,
    description: 'Successfully deleted the message.',
    type: DeleteMessageResponseDto,
  })
  @ApiOperation({ summary: 'Delete message', description: "Deletes a specific message defined by a message's `id`." })
  @ApiParam({ name: 'id', required: true, description: "The unique identifier of the message." })
  @Delete(':id')
  remove(@Param('id') id: string) {
    return this.messagesUsecases.remove(id);
  }
}
