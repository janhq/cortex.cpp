import { Inject, Injectable } from '@nestjs/common';
import { CreateModelDto } from '../../infrastructure/dtos/models/create-model.dto';
import { UpdateModelDto } from '../../infrastructure/dtos/models/update-model.dto';
import { ModelEntity } from '../../infrastructure/entities/model.entity';
import { Repository } from 'typeorm';
import { Model } from 'src/domain/models/model.interface';

@Injectable()
export class ModelsUsecases {
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
    return this.modelRepository.update(id, updateModelDto);
  }

  async remove(id: string) {
    return this.modelRepository.delete(id);
  }
}
