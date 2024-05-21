import {
  Controller,
  Get,
  Post,
  Body,
  Patch,
  Param,
  Delete,
  HttpCode
} from '@nestjs/common';
import { ModelsUsecases } from '@/usecases/models/models.usecases';
import { CreateModelDto } from '@/infrastructure/dtos/models/create-model.dto';
import { UpdateModelDto } from '@/infrastructure/dtos/models/update-model.dto';
import {
  ApiCreatedResponse,
  ApiOkResponse,
  ApiOperation,
  ApiParam,
  ApiTags,
  ApiResponse
} from '@nestjs/swagger';
import { StartModelSuccessDto } from '@/infrastructure/dtos/models/start-model-success.dto';
import { ModelSettingParamsDto } from '../dtos/models/model-setting-params.dto';

@ApiTags('Models')
@Controller('models')
export class ModelsController {
  constructor(private readonly modelsUsecases: ModelsUsecases) {}

  @ApiOperation({ summary: 'Create model', description: "Creates a model `.json` instance file manually." })
  @HttpCode(200)
  @ApiResponse({
    status: 200,
    description: 'The model has been successfully created.'
  })
  @Post()
  create(@Body() createModelDto: CreateModelDto) {
    return this.modelsUsecases.create(createModelDto);
  }

  @HttpCode(200)
  @ApiResponse({
    status: 200,
    description: 'The model has been successfully started.',
    type: StartModelSuccessDto,
  })
  @ApiOperation({ summary: 'Start model', description: "Start a model defined by a model `id`." })
  @ApiParam({ name: 'modelId', required: true, description: "The unique identifier of the model." })
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
    description: 'The model has been successfully stopped.',
    type: StartModelSuccessDto,
  })
  @ApiOperation({ summary: 'Stop model', description: "Stop a model defined by a model `id`." })
  @ApiParam({ name: 'modelId', required: true, description: "The unique identifier of the model." })
  @Post(':modelId/stop')
  stopModel(@Param('modelId') modelId: string) {
    return this.modelsUsecases.stopModel(modelId);
  }

  @ApiOperation({ summary: 'Download model', description: "Download a specific model instance." })
  @ApiParam({ name: 'modelId', required: true, description: "The unique identifier of the model." })
  @Get('download/:modelId')
  downloadModel(@Param('modelId') modelId: string) {
    return this.modelsUsecases.downloadModel(modelId);
  }

  @ApiOperation({ summary: 'List models', description: "Lists the currently available models, and provides basic information about each one such as the owner and availability. [Equivalent to OpenAI's list model](https://platform.openai.com/docs/api-reference/models/list)." })
  @Get()
  findAll() {
    return this.modelsUsecases.findAll();
  }

  @ApiOperation({ summary: 'Retrieve model', description: "Gets a model instance, providing basic information about the model such as the owner and permissions. [Equivalent to OpenAI's list model](https://platform.openai.com/docs/api-reference/models/retrieve)." })
  @ApiParam({ name: 'id', required: true, description: "The unique identifier of the model." })
  @Get(':id')
  findOne(@Param('id') id: string) {
    return this.modelsUsecases.findOne(id);
  }

  @HttpCode(200)
  @ApiResponse({
    status: 200,
    description: 'The model has been successfully updated.',
    type: UpdateModelDto,
  })
  @ApiOperation({ summary: 'Update model', description: "Updates a model instance defined by a mode;'s `id`." })
  @ApiParam({ name: 'id', required: true, description: "The unique identifier of the model." })
  @Patch(':id')
  update(@Param('id') id: string, @Body() updateModelDto: UpdateModelDto) {
    return this.modelsUsecases.update(id, updateModelDto);
  }

  @ApiResponse({
    status: 200,
    description: 'The model has been successfully deleted.'
  })
  @ApiOperation({ summary: 'Delete model', description: "Deletes a model. [Equivalent to OpenAI's delete model](https://platform.openai.com/docs/api-reference/models/delete)." })
  @ApiParam({ name: 'id', required: true, description: "The unique identifier of the model." })
  @Delete(':id')
  remove(@Param('id') id: string) {
    return this.modelsUsecases.remove(id);
  }
}
