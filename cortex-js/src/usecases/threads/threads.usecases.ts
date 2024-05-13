import { Inject, Injectable } from '@nestjs/common';
import { CreateThreadDto } from '@/infrastructure/dtos/threads/create-thread.dto';
import { UpdateThreadDto } from '@/infrastructure/dtos/threads/update-thread.dto';
import { ThreadEntity } from '@/infrastructure/entities/thread.entity';
import { Repository } from 'typeorm';

@Injectable()
export class ThreadsUsecases {
  constructor(
    @Inject('THREAD_REPOSITORY')
    private threadRepository: Repository<ThreadEntity>,
  ) {}

  async create(createThreadDto: CreateThreadDto): Promise<ThreadEntity> {
    const id = `jan_${(Date.now() / 1000).toFixed(0)}`;

    const thread: ThreadEntity = {
      ...createThreadDto,
      id,
      object: 'thread',
      createdAt: Date.now(),
    };
    await this.threadRepository.insert(thread);
    return thread;
  }

  async findAll(): Promise<ThreadEntity[]> {
    return this.threadRepository.find();
  }

  findOne(id: string) {
    return this.threadRepository.findOne({ where: { id } });
  }

  update(id: string, updateThreadDto: UpdateThreadDto) {
    return this.threadRepository.update(id, updateThreadDto);
  }

  remove(id: string) {
    this.threadRepository.delete(id);
  }
}
