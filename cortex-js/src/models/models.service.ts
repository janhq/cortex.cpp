import { Inject, Injectable } from '@nestjs/common';
import { CreateModelDto } from './dto/create-model.dto';
import { UpdateModelDto } from './dto/update-model.dto';
import { ModelEntity } from './entities/model.entity';
import { Repository } from 'typeorm';
import { Model } from 'src/core/interfaces/model.interface';

@Injectable()
export class ModelsService {
  constructor(
    @Inject('MODEL_REPOSITORY')
    private modelRepository: Repository<ModelEntity>,
  ) {}

  create(createModelDto: CreateModelDto) {
    const model: Model = {
      ...createModelDto,
      object: 'model',
      created: Date.now(),
    };

    this.modelRepository.insert(model);
  }

  async findAll(): Promise<Model[]> {
    return this.modelRepository.find();
  }

  async findOne(id: string) {
    return this.modelRepository.findOne({
      where: {
        id,
      },
    });
  }

  update(id: string, updateModelDto: UpdateModelDto) {
    return `This action updates a #${id} model`;
  }

  async remove(id: string) {
    // TODO: NamH not working
    this.modelRepository.delete(`id = ${id}`);
  }
}
