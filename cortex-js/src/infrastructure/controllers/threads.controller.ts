import {
  Controller,
  Get,
  Post,
  Body,
  Patch,
  Param,
  Delete,
} from '@nestjs/common';
import { ThreadsUsecases } from '@/usecases/threads/threads.usecases';
import { CreateThreadDto } from '@/infrastructure/dtos/threads/create-thread.dto';
import { UpdateThreadDto } from '@/infrastructure/dtos/threads/update-thread.dto';
import { ApiTags, ApiOperation } from '@nestjs/swagger';

@ApiTags('Threads')
@Controller('threads')
export class ThreadsController {
  constructor(private readonly threadsService: ThreadsUsecases) {}

  @ApiOperation({ summary: 'Create Thread', description: "This endpoint creates a new thread." })
  @Post()
  create(@Body() createThreadDto: CreateThreadDto) {
    return this.threadsService.create(createThreadDto);
  }

  @ApiOperation({ summary: 'List Threads', description: "Lists all the available threads along with its configurations." })
  @Get()
  findAll() {
    return this.threadsService.findAll();
  }

  @ApiOperation({ summary: 'Get Thread', description: "Retrieves a thread along with its configurations." })
  @Get(':id')
  findOne(@Param('id') id: string) {
    return this.threadsService.findOne(id);
  }

  @ApiOperation({ summary: 'Update Thread', description: "Updates a thread's configurations." })
  @Patch(':id')
  update(@Param('id') id: string, @Body() updateThreadDto: UpdateThreadDto) {
    return this.threadsService.update(id, updateThreadDto);
  }

  @ApiOperation({ summary: 'Delete Thread', description: "Deletes a specific thread defined by a thread `id` ." })
  @Delete(':id')
  remove(@Param('id') id: string) {
    return this.threadsService.remove(id);
  }
}
