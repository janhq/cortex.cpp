import {
  Controller,
  Get,
  Post,
  Body,
  Param,
  Delete,
  UseInterceptors,
  Query,
  DefaultValuePipe,
} from '@nestjs/common';
import { ThreadsUsecases } from '@/usecases/threads/threads.usecases';
import { CreateThreadDto } from '@/infrastructure/dtos/threads/create-thread.dto';
import { UpdateThreadDto } from '@/infrastructure/dtos/threads/update-thread.dto';
import { DeleteThreadResponseDto } from '@/infrastructure/dtos/threads/delete-thread.dto';
import { GetThreadResponseDto } from '@/infrastructure/dtos/threads/get-thread.dto';
import { ApiOkResponse, ApiOperation, ApiResponse } from '@nestjs/swagger';
import { TransformInterceptor } from '../interceptors/transform.interceptor';
import { ListMessagesResponseDto } from '../dtos/messages/list-message.dto';
import { CreateMessageDto } from '../dtos/threads/create-message.dto';
import { UpdateMessageDto } from '../dtos/threads/update-message.dto';
import DeleteMessageDto from '../dtos/threads/delete-message.dto';
import { GetMessageResponseDto } from '../dtos/messages/get-message.dto';

@Controller('threads')
@UseInterceptors(TransformInterceptor)
export class ThreadsController {
  constructor(private readonly threadsUsecases: ThreadsUsecases) {}

  @ApiOperation({
    tags: ['Threads'],
    summary: 'Create thread',
    description: 'Creates a new thread.',
  })
  @Post()
  create(@Body() createThreadDto: CreateThreadDto) {
    return this.threadsUsecases.create(createThreadDto);
  }

  @ApiOperation({
    tags: ['Threads'],
    summary: 'List threads',
    description:
      'Lists all the available threads along with its configurations.',
  })
  @Get()
  findAll() {
    return this.threadsUsecases.findAll();
  }

  @ApiOperation({
    tags: ['Messages'],
    summary: 'Retrieve message',
    description: 'Retrieves a message.',
    parameters: [
      {
        in: 'path',
        name: 'thread_id',
        required: true,
        description: 'The ID of the thread to which this message belongs.',
      },
      {
        in: 'path',
        name: 'message_id',
        required: true,
        description: 'The ID of the message to retrieve.',
      },
    ],
  })
  @ApiOkResponse({
    description: 'The message object matching the specified ID.',
    type: GetMessageResponseDto,
  })
  @Get(':thread_id/messages/:message_id')
  async retrieveMessage(
    @Param('thread_id') threadId: string,
    @Param('message_id') messageId: string,
  ) {
    return this.threadsUsecases.retrieveMessage(threadId, messageId);
  }

  @ApiOperation({
    tags: ['Messages'],
    summary: 'List messages',
    description: 'Returns a list of messages for a given thread.',
    parameters: [
      {
        in: 'path',
        name: 'thread_id',
        required: true,
        description: 'The ID of the thread the messages belong to.',
      },
      {
        in: 'query',
        name: 'limit',
        required: false,
        description:
          'A limit on the number of objects to be returned. Limit can range between 1 and 100, and the default is 20.',
      },
      {
        in: 'query',
        name: 'order',
        required: false,
        description:
          'Sort order by the created_at timestamp of the objects. asc for ascending order and desc for descending order.',
      },
      {
        in: 'query',
        name: 'after',
        required: false,
        description:
          'A cursor for use in pagination. after is an object ID that defines your place in the list. For instance, if you make a list request and receive 100 objects, ending with obj_foo, your subsequent call can include after=obj_foo in order to fetch the next page of the list.',
      },
      {
        in: 'query',
        name: 'before',
        required: false,
        description:
          'A cursor for use in pagination. before is an object ID that defines your place in the list. For instance, if you make a list request and receive 100 objects, ending with obj_foo, your subsequent call can include before=obj_foo in order to fetch the previous page of the list.',
      },
      {
        in: 'query',
        name: 'run_id',
        required: false,
        description: 'Filter messages by the run ID that generated them.',
      },
    ],
  })
  @ApiOkResponse({
    description: 'A list of message objects.',
    type: ListMessagesResponseDto,
  })
  @Get(':thread_id/messages')
  getMessagesOfThread(
    @Param('thread_id') threadId: string,
    @Query('limit', new DefaultValuePipe(20)) limit: number,
    @Query('order', new DefaultValuePipe('desc')) order: 'asc' | 'desc',
    @Query('after') after?: string,
    @Query('before') before?: string,
    @Query('run_id') runId?: string,
  ) {
    return this.threadsUsecases.getMessagesOfThread(
      threadId,
      limit,
      order,
      after,
      before,
      runId,
    );
  }

  @ApiOperation({
    tags: ['Messages'],
    summary: 'Create message',
    description: 'Create a message.',
    responses: {
      '201': {
        description: 'A message object.',
      },
    },
    parameters: [
      {
        in: 'path',
        name: 'thread_id',
        required: true,
        description: 'The ID of the thread to create a message for.',
      },
    ],
  })
  @Post(':thread_id/messages')
  createMessageInThread(
    @Param('thread_id') threadId: string,
    @Body() createMessageDto: CreateMessageDto,
  ) {
    return this.threadsUsecases.createMessageInThread(
      threadId,
      createMessageDto,
    );
  }

  @ApiOperation({
    tags: ['Messages'],
    summary: 'Modify message',
    description: 'Modifies a message.',
    responses: {
      '200': {
        description: 'The modified message object.',
      },
    },
    parameters: [
      {
        in: 'path',
        name: 'thread_id',
        required: true,
        description: 'The ID of the thread to which this message belongs.',
      },
      {
        in: 'path',
        name: 'message_id',
        required: true,
        description: 'The ID of the message to modify.',
      },
    ],
  })
  @Post(':thread_id/messages/:message_id')
  updateMessage(
    @Param('thread_id') threadId: string,
    @Param('message_id') messageId: string,
    @Body() updateMessageDto: UpdateMessageDto,
  ) {
    return this.threadsUsecases.updateMessage(
      threadId,
      messageId,
      updateMessageDto,
    );
  }

  @ApiOperation({
    summary: 'Clean thread',
    description: 'Deletes all messages in a thread.',
    tags: ['Threads'],
    parameters: [
      {
        in: 'path',
        name: 'thread_id',
        required: true,
        description: 'The ID of the thread to clean.',
      },
    ],
  })
  @Post(':thread_id/clean')
  async cleanThread(@Param('thread_id') threadId: string) {
    return this.threadsUsecases.clean(threadId);
  }

  @ApiOperation({
    summary: 'Delete message',
    description: 'Deletes a message.',
    tags: ['Messages'],
    parameters: [
      {
        in: 'path',
        name: 'thread_id',
        required: true,
        description: 'The ID of the thread to which this message belongs.',
      },
      {
        in: 'path',
        name: 'message_id',
        required: true,
        description: 'The ID of the message to delete.',
      },
    ],
  })
  @ApiOkResponse({
    description: 'Deletion status.',
    type: DeleteMessageDto,
  })
  @Delete(':thread_id/messages/:message_id')
  deleteMessage(
    @Param('thread_id') threadId: string,
    @Param('message_id') messageId: string,
  ) {
    return this.threadsUsecases.deleteMessage(threadId, messageId);
  }

  @ApiOperation({
    tags: ['Threads'],
    summary: 'Retrieve thread',
    description: 'Retrieves a thread.',
    parameters: [
      {
        in: 'path',
        name: 'thread_id',
        required: true,
        description: 'The unique identifier of the thread.',
      },
    ],
  })
  @ApiOkResponse({
    description: 'Retrieves a thread.',
    type: GetThreadResponseDto,
  })
  @Get(':thread_id')
  retrieveThread(@Param('thread_id') threadId: string) {
    return this.threadsUsecases.findOne(threadId);
  }

  @ApiResponse({
    status: 200,
    description: 'The thread has been successfully updated.',
    type: UpdateThreadDto,
  })
  @ApiOperation({
    tags: ['Threads'],
    summary: 'Modify thread',
    description: 'Modifies a thread.',
    parameters: [
      {
        in: 'path',
        name: 'thread_id',
        required: true,
        description: 'The unique identifier of the thread.',
      },
    ],
  })
  @Post(':thread_id')
  modifyThread(
    @Param('thread_id') threadId: string,
    @Body() updateThreadDto: UpdateThreadDto,
  ) {
    return this.threadsUsecases.update(threadId, updateThreadDto);
  }

  @ApiResponse({
    status: 200,
    description: 'The thread has been successfully deleted.',
    type: DeleteThreadResponseDto,
  })
  @ApiOperation({
    tags: ['Threads'],
    summary: 'Delete thread',
    description: 'Deletes a specific thread defined by a thread `id` .',
    parameters: [
      {
        in: 'path',
        name: 'id',
        required: true,
        description: 'The unique identifier of the thread.',
      },
    ],
  })
  @Delete(':thread_id')
  remove(@Param('thread_id') threadId: string) {
    return this.threadsUsecases.remove(threadId);
  }
}
