import {
  Controller,
  Get,
  Post,
  Body,
  Patch,
  Param,
  Delete,
  HttpCode,
  UseInterceptors,
} from '@nestjs/common';
import { ModelsUsecases } from '@/usecases/models/models.usecases';
import { CreateModelDto } from '@/infrastructure/dtos/models/create-model.dto';
import { UpdateModelDto } from '@/infrastructure/dtos/models/update-model.dto';
import { ApiResponse, ApiTags } from '@nestjs/swagger';
import { StartModelSuccessDto } from '@/infrastructure/dtos/models/start-model-success.dto';
import { ModelSettingParamsDto } from '../dtos/models/model-setting-params.dto';
import { TransformInterceptor } from '../interceptors/transform.interceptor';

@ApiTags('Models')
@Controller('models')
@UseInterceptors(TransformInterceptor)
export class ModelsController {
  constructor(private readonly modelsUsecases: ModelsUsecases) {}

  @Post()
  create(@Body() createModelDto: CreateModelDto) {
    return this.modelsUsecases.create(createModelDto);
  }

  @HttpCode(200)
  @ApiResponse({
    status: 200,
    description: 'The model has been started successfully.',
    type: StartModelSuccessDto,
  })
  @Post(':modelId/start')
  startModel(
    @Param('modelId') modelId: string,
    @Body() settings: ModelSettingParamsDto,
  ) {
    return this.modelsUsecases.startModel(modelId, settings);
  }

  @HttpCode(200)
  @ApiResponse({
    status: 200,
    description: 'The model has been stopped successfully.',
    type: StartModelSuccessDto,
  })
  @Post(':modelId/stop')
  stopModel(@Param('modelId') modelId: string) {
    return this.modelsUsecases.stopModel(modelId);
  }

  @Get('download/:modelId')
  downloadModel(@Param('modelId') modelId: string) {
    return this.modelsUsecases.downloadModel(modelId);
  }

  @Get()
  findAll() {
    return this.modelsUsecases.findAll();
  }

  @Get(':id')
  findOne(@Param('id') id: string) {
    return this.modelsUsecases.findOne(id);
  }

  @Patch(':id')
  update(@Param('id') id: string, @Body() updateModelDto: UpdateModelDto) {
    return this.modelsUsecases.update(id, updateModelDto);
  }

  @Delete(':id')
  remove(@Param('id') id: string) {
    return this.modelsUsecases.remove(id);
  }
}
