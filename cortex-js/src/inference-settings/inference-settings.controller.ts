import {
  Controller,
  Get,
  Post,
  Body,
  Patch,
  Param,
  Delete,
} from '@nestjs/common';
import { InferenceSettingsService } from './inference-settings.service';
import { CreateInferenceSettingDto } from './dto/create-inference-setting.dto';
import { UpdateInferenceSettingDto } from './dto/update-inference-setting.dto';

@Controller('inference-settings')
export class InferenceSettingsController {
  constructor(
    private readonly inferenceSettingsService: InferenceSettingsService,
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
