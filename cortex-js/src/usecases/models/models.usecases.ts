import { CreateModelDto } from '../../infrastructure/dtos/models/create-model.dto';
import { UpdateModelDto } from '../../infrastructure/dtos/models/update-model.dto';
import { ModelEntity } from '../../infrastructure/entities/model.entity';
import { BadRequestException, Inject, Injectable } from '@nestjs/common';
import { Repository } from 'typeorm';
import { Model, ModelFormat } from 'src/domain/models/model.interface';
import { ModelNotFoundException } from 'src/infrastructure/exception/model-not-found.exception';
import { join, basename, resolve } from 'path';
import { createWriteStream, promises } from 'fs';
import { LoadModelSuccessDto } from 'src/infrastructure/dtos/models/load-model-success.dto';
import { LoadModelDto } from 'src/infrastructure/dtos/models/load-model.dto';
import { DownloadModelDto } from 'src/infrastructure/dtos/models/download-model.dto';
import { ExtensionRepository } from '@/domain/repositories/extension.interface';
import { EngineExtension } from '@janhq/core';

@Injectable()
export class ModelsUsecases {
  constructor(
    @Inject('MODEL_REPOSITORY')
    private readonly modelRepository: Repository<ModelEntity>,
    private readonly extensionRepository: ExtensionRepository,
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

  async loadModel(loadModelDto: LoadModelDto): Promise<LoadModelSuccessDto> {
    const model = await this.findOne(loadModelDto.modelId);

    if (!model) {
      return {
        message: 'Model failed to load',
        modelId: loadModelDto.modelId,
      };
    }
    const extensions = (await this.extensionRepository.findAll()) ?? [];

    const engine = extensions.find((e: any) => e.provider === model?.engine) as
      | EngineExtension
      | undefined;

    if (!engine) {
      return {
        message: 'No extension handler found for model',
        modelId: loadModelDto.modelId,
      };
    }

    return engine
      .loadModel(loadModelDto)
      .then(() => {
        return {
          message: 'Model failed to load',
          modelId: loadModelDto.modelId,
        };
      })
      .catch(() => {
        return {
          message: 'Model loaded successfully',
          modelId: loadModelDto.modelId,
        };
      });
  }

  async downloadModel(downloadModelDto: DownloadModelDto) {
    const model = await this.findOne(downloadModelDto.modelId);
    if (!model) {
      throw new ModelNotFoundException(downloadModelDto.modelId);
    }

    if (model.format === ModelFormat.API) {
      throw new BadRequestException('Cannot download remote model');
    }

    // TODO: NamH download multiple files

    const downloadUrl = model.sources[0].url;
    if (!downloadUrl || !downloadUrl.startsWith('http')) {
      throw new BadRequestException('Invalid download URL');
    }
    const fileName = basename(downloadUrl);

    await promises.mkdir(join(fileName, '..'), { recursive: true });

    // eslint-disable-next-line @typescript-eslint/no-var-requires
    const fetch = require('node-fetch');
    const response = await fetch(downloadUrl);
    const destination = resolve('./downloads', fileName);
    const fileStream = createWriteStream(destination);
    response.body.pipe(fileStream);
  }
}
