import { Inject, Injectable } from '@nestjs/common';
import { CreateThreadDto } from './dto/create-thread.dto';
import { UpdateThreadDto } from './dto/update-thread.dto';
import { Thread } from './entities/thread.entity';
import { Repository } from 'typeorm';

@Injectable()
export class ThreadsService {
  constructor(
    @Inject('THREAD_REPOSITORY') private threadRepository: Repository<Thread>,
  ) {}

  async create(createThreadDto: CreateThreadDto): Promise<Thread> {
    const { title } = createThreadDto;
    const id = `jan_${(Date.now() / 1000).toFixed(0)}`;

    const thread: Thread = {
      id,
      object: 'thread',
      title,
      createdAt: Date.now(),
    };
    await this.threadRepository.insert(thread);
    return thread;
  }

  async findAll(): Promise<Thread[]> {
    return this.threadRepository.find();
  }

  findOne(id: number) {
    return `This action returns a #${id} thread`;
  }

  update(id: number, updateThreadDto: UpdateThreadDto) {
    return `This action updates a #${id} thread`;
  }

  remove(id: number) {
    return `This action removes a #${id} thread`;
  }
}
