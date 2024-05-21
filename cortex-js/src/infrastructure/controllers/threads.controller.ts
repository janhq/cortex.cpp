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
import { ThreadsUsecases } from '@/usecases/threads/threads.usecases';
import { CreateThreadDto } from '@/infrastructure/dtos/threads/create-thread.dto';
import { UpdateThreadDto } from '@/infrastructure/dtos/threads/update-thread.dto';
import { ApiTags } from '@nestjs/swagger';
import { TransformInterceptor } from '../interceptors/transform.interceptor';

@ApiTags('Threads')
@Controller('threads')
@UseInterceptors(TransformInterceptor)
export class ThreadsController {
  constructor(private readonly threadsService: ThreadsUsecases) {}

  @Post()
  create(@Body() createThreadDto: CreateThreadDto) {
    return this.threadsService.create(createThreadDto);
  }

  @Get()
  findAll() {
    return this.threadsService.findAll();
  }

  @Get(':id')
  findOne(@Param('id') id: string) {
    return this.threadsService.findOne(id);
  }

  @Patch(':id')
  update(@Param('id') id: string, @Body() updateThreadDto: UpdateThreadDto) {
    return this.threadsService.update(id, updateThreadDto);
  }

  @Delete(':id')
  remove(@Param('id') id: string) {
    return this.threadsService.remove(id);
  }
}
