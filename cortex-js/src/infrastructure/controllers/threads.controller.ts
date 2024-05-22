import {
  Controller,
  Get,
  Post,
  Body,
  Patch,
  Param,
  Delete,
  HttpCode
} from '@nestjs/common';
import { ThreadsUsecases } from '@/usecases/threads/threads.usecases';
import { CreateThreadDto } from '@/infrastructure/dtos/threads/create-thread.dto';
import { UpdateThreadDto } from '@/infrastructure/dtos/threads/update-thread.dto';
import { DeleteThreadResponseDto } from '@/infrastructure/dtos/threads/delete-thread.dto';
import { GetThreadResponseDto } from '@/infrastructure/dtos/threads/get-thread.dto';
import {
  ApiCreatedResponse,
  ApiOkResponse,
  ApiOperation,
  ApiParam,
  ApiTags,
  ApiResponse
} from '@nestjs/swagger';

@ApiTags('Threads')
@Controller('threads')
export class ThreadsController {
  constructor(private readonly threadsService: ThreadsUsecases) {}

  @ApiOperation({ summary: 'Create thread', description: "Creates a new thread." })
  @Post()
  create(@Body() createThreadDto: CreateThreadDto) {
    return this.threadsService.create(createThreadDto);
  }

  @ApiOperation({ summary: 'List threads', description: "Lists all the available threads along with its configurations." })
  @Get()
  findAll() {
    return this.threadsService.findAll();
  }

  @ApiResponse({
    status: 200,
    description: 'Ok',
    type: GetThreadResponseDto,
  })
  @ApiOperation({ summary: 'Get thread', description: "Retrieves a thread along with its configurations." })
  @ApiParam({ name: 'id', required: true, description: "The unique identifier of the thread." })
  @Get(':id')
  findOne(@Param('id') id: string) {
    return this.threadsService.findOne(id);
  }

  @ApiResponse({
    status: 200,
    description: 'The thread has been successfully updated.',
    type: UpdateThreadDto,
  })
  @ApiOperation({ summary: 'Update thread', description: "Updates a thread's configurations." })
  @ApiParam({ name: 'id', required: true, description: "The unique identifier of the thread." })
  @Patch(':id')
  update(@Param('id') id: string, @Body() updateThreadDto: UpdateThreadDto) {
    return this.threadsService.update(id, updateThreadDto);
  }

  @ApiResponse({
    status: 200,
    description: 'The thread has been successfully deleted.',
    type: DeleteThreadResponseDto,
  })
  @ApiOperation({ summary: 'Delete thread', description: "Deletes a specific thread defined by a thread `id` ." })
  @ApiParam({ name: 'id', required: true, description: "The unique identifier of the thread." })
  @Delete(':id')
  remove(@Param('id') id: string) {
    return this.threadsService.remove(id);
  }
}
