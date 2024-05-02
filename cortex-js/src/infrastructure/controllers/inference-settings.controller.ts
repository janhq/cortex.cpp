import {
  Controller,
  Get,
  Post,
  Body,
  Patch,
  Param,
  Delete,
} from '@nestjs/common';
import { InferenceSettingsUsecases } from '../../usecases/inference-settings/inference-settings.usecases';
import { CreateInferenceSettingDto } from '../dtos/inference-settings/create-inference-setting.dto';
import { UpdateInferenceSettingDto } from '../dtos/inference-settings/update-inference-setting.dto';
import { ApiTags } from '@nestjs/swagger';

@ApiTags('Inference Settings')
@Controller('inference-settings')
export class InferenceSettingsController {
  constructor(
    private readonly inferenceSettingsService: InferenceSettingsUsecases,
  ) {}

  @Post()
  create(@Body() createInferenceSettingDto: CreateInferenceSettingDto) {
    return this.inferenceSettingsService.create(createInferenceSettingDto);
  }

  @Get()
  findAll() {
    return this.inferenceSettingsService.findAll();
  }

  @Get(':id')
  findOne(@Param('id') id: string) {
    return this.inferenceSettingsService.findOne(id);
  }

  @Patch(':id')
  update(
    @Param('id') id: string,
    @Body() updateInferenceSettingDto: UpdateInferenceSettingDto,
  ) {
    return this.inferenceSettingsService.update(id, updateInferenceSettingDto);
  }

  @Delete(':id')
  remove(@Param('id') id: string) {
    return this.inferenceSettingsService.remove(id);
  }
}
