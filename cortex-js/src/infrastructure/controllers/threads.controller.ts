import {
  Controller,
  Get,
  Post,
  Body,
  Patch,
  Param,
  Delete,
  UseInterceptors,
  HttpCode,
  Query,
  DefaultValuePipe,
} from '@nestjs/common';
import { ThreadsUsecases } from '@/usecases/threads/threads.usecases';
import { CreateThreadDto } from '@/infrastructure/dtos/threads/create-thread.dto';
import { UpdateThreadDto } from '@/infrastructure/dtos/threads/update-thread.dto';
import { DeleteThreadResponseDto } from '@/infrastructure/dtos/threads/delete-thread.dto';
import { GetThreadResponseDto } from '@/infrastructure/dtos/threads/get-thread.dto';
import {
  ApiOperation,
  ApiParam,
  ApiTags,
  ApiResponse,
  ApiQuery,
} from '@nestjs/swagger';
import { TransformInterceptor } from '../interceptors/transform.interceptor';
import { ListMessagesResponseDto } from '../dtos/messages/list-message.dto';

@ApiTags('Threads')
@Controller('threads')
@UseInterceptors(TransformInterceptor)
export class ThreadsController {
  constructor(private readonly threadsService: ThreadsUsecases) {}

  @ApiOperation({
    summary: 'Create thread',
    description: 'Creates a new thread.',
  })
  @Post()
  create(@Body() createThreadDto: CreateThreadDto) {
    return this.threadsService.create(createThreadDto);
  }

  @ApiOperation({
    summary: 'List threads',
    description:
      'Lists all the available threads along with its configurations.',
  })
  @Get()
  findAll() {
    return this.threadsService.findAll();
  }

  @HttpCode(200)
  @ApiResponse({
    status: 200,
    description: 'A list of message objects.',
    type: ListMessagesResponseDto,
  })
  @ApiOperation({
    summary: 'List messages',
    description: 'Returns a list of messages for a given thread.',
  })
  @ApiParam({
    name: 'id',
    required: true,
    description: 'The ID of the thread the messages belong to.',
  })
  @ApiQuery({
    name: 'limit',
    type: Number,
    required: false,
    description:
      'A limit on the number of objects to be returned. Limit can range between 1 and 100, and the default is 20.',
  })
  @ApiQuery({
    name: 'order',
    type: String,
    required: false,
    description:
      'Sort order by the created_at timestamp of the objects. asc for ascending order and desc for descending order.',
  })
  @ApiQuery({
    name: 'after',
    type: String,
    required: false,
    description:
      'A cursor for use in pagination. after is an object ID that defines your place in the list. For instance, if you make a list request and receive 100 objects, ending with obj_foo, your subsequent call can include after=obj_foo in order to fetch the next page of the list.',
  })
  @ApiQuery({
    name: 'before',
    type: String,
    required: false,
    description:
      'A cursor for use in pagination. before is an object ID that defines your place in the list. For instance, if you make a list request and receive 100 objects, ending with obj_foo, your subsequent call can include before=obj_foo in order to fetch the previous page of the list.',
  })
  @ApiQuery({
    name: 'run_id',
    type: String,
    required: false,
    description: 'Filter messages by the run ID that generated them.',
  })
  @Get(':id/messages')
  getMessagesOfThread(
    @Param('id') id: string,
    @Query('limit', new DefaultValuePipe(20)) limit: number,
    @Query('order', new DefaultValuePipe('desc')) order: 'asc' | 'desc',
    @Query('after') after?: string,
    @Query('before') before?: string,
    @Query('run_id') runId?: string,
  ) {
    return this.threadsService.getMessagesOfThread(
      id,
      limit,
      order,
      after,
      before,
      runId,
    );
  }

  @ApiResponse({
    status: 200,
    description: 'Ok',
    type: GetThreadResponseDto,
  })
  @ApiOperation({
    summary: 'Get thread',
    description: 'Retrieves a thread along with its configurations.',
  })
  @ApiParam({
    name: 'id',
    required: true,
    description: 'The unique identifier of the thread.',
  })
  @Get(':id')
  findOne(@Param('id') id: string) {
    return this.threadsService.findOne(id);
  }

  @ApiResponse({
    status: 200,
    description: 'The thread has been successfully updated.',
    type: UpdateThreadDto,
  })
  @ApiOperation({
    summary: 'Update thread',
    description: "Updates a thread's configurations.",
  })
  @ApiParam({
    name: 'id',
    required: true,
    description: 'The unique identifier of the thread.',
  })
  @Patch(':id')
  update(@Param('id') id: string, @Body() updateThreadDto: UpdateThreadDto) {
    return this.threadsService.update(id, updateThreadDto);
  }

  @ApiResponse({
    status: 200,
    description: 'The thread has been successfully deleted.',
    type: DeleteThreadResponseDto,
  })
  @ApiOperation({
    summary: 'Delete thread',
    description: 'Deletes a specific thread defined by a thread `id` .',
  })
  @ApiParam({
    name: 'id',
    required: true,
    description: 'The unique identifier of the thread.',
  })
  @Delete(':id')
  remove(@Param('id') id: string) {
    return this.threadsService.remove(id);
  }
}
