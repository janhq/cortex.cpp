import {
  Controller,
  Get,
  Post,
  Body,
  Patch,
  Param,
  Delete,
  HttpCode,
} from '@nestjs/common';
import { ModelsUsecases } from '@/usecases/models/models.usecases';
import { CreateModelDto } from '@/infrastructure/dtos/models/create-model.dto';
import { UpdateModelDto } from '@/infrastructure/dtos/models/update-model.dto';
import { ApiResponse, ApiTags, ApiOperation } from '@nestjs/swagger';
import { StartModelSuccessDto } from '@/infrastructure/dtos/models/start-model-success.dto';
import { ModelSettingParamsDto } from '../dtos/models/model-setting-params.dto';

@ApiTags('Models')
@Controller('models')
export class ModelsController {
  constructor(private readonly modelsUsecases: ModelsUsecases) {}

  @ApiOperation({ summary: 'Create Model', description: "Creates a model `.json` instance file manually." })
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
  @ApiOperation({ summary: 'Start Model', description: "Start a model defined by a model `id`." })
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
  @ApiOperation({ summary: 'Stop Model', description: "Stop a model defined by a model `id`." })
  @Post(':modelId/stop')
  stopModel(@Param('modelId') modelId: string) {
    return this.modelsUsecases.stopModel(modelId);
  }

  @ApiOperation({ summary: 'Download Model', description: "Download a specific model instance." })
  @Get('download/:modelId')
  downloadModel(@Param('modelId') modelId: string) {
    return this.modelsUsecases.downloadModel(modelId);
  }

  @ApiOperation({ summary: 'List Models', description: "Lists the currently available models, and provides basic information about each one such as the owner and availability. [Equivalent to OpenAI's list model](https://platform.openai.com/docs/api-reference/models/list)." })
  @Get()
  findAll() {
    return this.modelsUsecases.findAll();
  }

  @ApiOperation({ summary: 'Retrieve Model', description: "Gets a model instance, providing basic information about the model such as the owner and permissioning. [Equivalent to OpenAI's list model](https://platform.openai.com/docs/api-reference/models/retrieve)." })
  @Get(':id')
  findOne(@Param('id') id: string) {
    return this.modelsUsecases.findOne(id);
  }

  @ApiOperation({ summary: 'Update Model', description: "Updates a model instance defined by a mode;'s `id`." })
  @Patch(':id')
  update(@Param('id') id: string, @Body() updateModelDto: UpdateModelDto) {
    return this.modelsUsecases.update(id, updateModelDto);
  }

  @ApiOperation({ summary: 'Delete Model', description: "Deletes a model. [Equivalent to OpenAI's delete model](https://platform.openai.com/docs/api-reference/models/delete)." })
  @Delete(':id')
  remove(@Param('id') id: string) {
    return this.modelsUsecases.remove(id);
  }
}
