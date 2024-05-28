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
import { ModelDto } from '@/infrastructure/dtos/models/model-successfully-created.dto';
import { ListModelsResponseDto } from '@/infrastructure/dtos/models/list-model-response.dto';
import { DeleteModelResponseDto } from '@/infrastructure/dtos/models/delete-model.dto';
import { DownloadModelResponseDto } from '@/infrastructure/dtos/models/download-model.dto';
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
import { TransformInterceptor } from '../interceptors/transform.interceptor';

@ApiTags('Models')
@Controller('models')
@UseInterceptors(TransformInterceptor)
export class ModelsController {
  constructor(private readonly modelsUsecases: ModelsUsecases) {}

  @HttpCode(201)
  @ApiResponse({
    status: 201,
    description: 'The model has been successfully created.',
    type: StartModelSuccessDto,
  })
  @ApiOperation({ summary: 'Create model', description: "Creates a model `.json` instance file manually." })
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
  @ApiOperation({ summary: 'Start model', description: "Starts a model operation defined by a model `id`." })
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
  @ApiOperation({ summary: 'Stop model', description: "Stops a model operation defined by a model `id`." })
  @ApiParam({ name: 'modelId', required: true, description: "The unique identifier of the model." })
  @Post(':modelId/stop')
  stopModel(@Param('modelId') modelId: string) {
    return this.modelsUsecases.stopModel(modelId);
  }

  @HttpCode(200)
  @ApiResponse({
    status: 200,
    description: 'Ok',
    type: DownloadModelResponseDto,
  })
  @ApiOperation({ summary: 'Download model', description: "Downloads a specific model instance." })
  @ApiParam({ name: 'modelId', required: true, description: "The unique identifier of the model." })
  @Get('download/:modelId')
  downloadModel(@Param('modelId') modelId: string) {
    return this.modelsUsecases.downloadModel(modelId);
  }

  @HttpCode(200)
  @ApiResponse({
    status: 200,
    description: 'Ok',
    type: ListModelsResponseDto,
  })
  @ApiOperation({ summary: 'List models', description: "Lists the currently available models, and provides basic information about each one such as the owner and availability. [Equivalent to OpenAI's list model](https://platform.openai.com/docs/api-reference/models/list)." })
  @Get()
  findAll() {
    return this.modelsUsecases.findAll();
  }

  @HttpCode(200)
  @ApiResponse({
    status: 200,
    description: 'Ok',
    type: ModelDto,
  })
  @ApiOperation({ summary: 'Get model', description: "Retrieves a model instance, providing basic information about the model such as the owner and permissions. [Equivalent to OpenAI's list model](https://platform.openai.com/docs/api-reference/models/retrieve)." })
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
  @ApiOperation({ summary: 'Update model', description: "Updates a model instance defined by a model's `id`." })
  @ApiParam({ name: 'id', required: true, description: "The unique identifier of the model." })
  @Patch(':id')
  update(@Param('id') id: string, @Body() updateModelDto: UpdateModelDto) {
    return this.modelsUsecases.update(id, updateModelDto);
  }

  @ApiResponse({
    status: 200,
    description: 'The model has been successfully deleted.',
    type: DeleteModelResponseDto,
  })
  @ApiOperation({ summary: 'Delete model', description: "Deletes a model. [Equivalent to OpenAI's delete model](https://platform.openai.com/docs/api-reference/models/delete)." })
  @ApiParam({ name: 'id', required: true, description: "The unique identifier of the model." })
  @Delete(':id')
  remove(@Param('id') id: string) {
    return this.modelsUsecases.remove(id);
  }
}
