import { CreateModelDto } from '../../infrastructure/dtos/models/create-model.dto';
import { UpdateModelDto } from '../../infrastructure/dtos/models/update-model.dto';
import { ModelEntity } from '../../infrastructure/entities/model.entity';
import {
  BadRequestException,
  Inject,
  Injectable,
  InternalServerErrorException,
} from '@nestjs/common';
import { Repository } from 'typeorm';
import { Model, ModelFormat } from 'src/domain/models/model.interface';
import { ModelNotFoundException } from 'src/infrastructure/exception/model-not-found.exception';
import { join, basename } from 'path';
import { createWriteStream, promises } from 'fs';
import { LoadModelSuccessDto } from 'src/infrastructure/dtos/models/load-model-success.dto';
import { LoadModelDto } from 'src/infrastructure/dtos/models/load-model.dto';
import { DownloadModelDto } from 'src/infrastructure/dtos/models/download-model.dto';
import { CortexUsecases } from '../cortex/cortex.usecases';
import { ConfigService } from '@nestjs/config';
import { ExtensionRepository } from '@/domain/repositories/extension.interface';
import { EngineExtension } from '@janhq/core';

@Injectable()
export class ModelsUsecases {
  constructor(
    @Inject('MODEL_REPOSITORY')
    private readonly modelRepository: Repository<ModelEntity>,
    private readonly extensionRepository: ExtensionRepository,
    private readonly cortexUsecases: CortexUsecases,
    private readonly configService: ConfigService,
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
    if (!this.isValidUrl(downloadUrl)) {
      throw new BadRequestException(`Invalid download URL: ${downloadUrl}`);
    }

    const fileName = basename(downloadUrl);
    const modelsContainerDir =
      this.configService.get<string>('CORTEX_MODELS_DIR');

    if (!modelsContainerDir) {
      throw new InternalServerErrorException(
        'CORTEX_MODELS_DIR is null. Recheck your environment variables to ensure all required directories are created',
      );
    }

    const modelFolder = join(modelsContainerDir, model.id);
    await promises.mkdir(modelFolder, { recursive: true });

    const destination = join(modelFolder, fileName);

    // eslint-disable-next-line @typescript-eslint/no-var-requires
    const fetch = require('node-fetch');
    const response = await fetch(downloadUrl);
    const fileStream = createWriteStream(destination);
    response.body.pipe(fileStream);
  }

  // TODO: NamH move to a helper or utils
  private isValidUrl(input: string | undefined): boolean {
    if (!input) return false;
    try {
      new URL(input);
      return true;
    } catch (e) {
      return false;
    }
  }
}
