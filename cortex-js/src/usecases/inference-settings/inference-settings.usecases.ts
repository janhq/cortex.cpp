import { Inject, Injectable } from '@nestjs/common';
import { CreateInferenceSettingDto } from '../../infrastructure/dtos/inference-settings/create-inference-setting.dto';
import { UpdateInferenceSettingDto } from '../../infrastructure/dtos/inference-settings/update-inference-setting.dto';
import { Repository } from 'typeorm';
import { InferenceSettingEntity } from '../../infrastructure/entities/inference-setting.entity';

@Injectable()
export class InferenceSettingsUsecases {
  constructor(
    @Inject('INFERENCE_SETTING_REPOSITORY')
    private inferenceSettingRepository: Repository<InferenceSettingEntity>,
  ) {}

  create(createInferenceSettingDto: CreateInferenceSettingDto) {
    return this.inferenceSettingRepository.insert(createInferenceSettingDto);
  }

  findAll() {
    return this.inferenceSettingRepository.find();
  }

  findOne(id: string) {
    return this.inferenceSettingRepository.findOne({
      where: { inferenceId: id },
    });
  }

  update(id: string, updateInferenceSettingDto: UpdateInferenceSettingDto) {
    return this.inferenceSettingRepository.update(
      id,
      updateInferenceSettingDto,
    );
  }

  remove(id: string) {
    return this.inferenceSettingRepository.delete(id);
  }
}
