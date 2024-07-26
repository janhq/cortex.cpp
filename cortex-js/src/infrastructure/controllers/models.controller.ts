import {
  Controller,
  Get,
  Post,
  Body,
  Param,
  Delete,
  HttpCode,
  UseInterceptors,
  BadRequestException,
  Patch,
} from '@nestjs/common';
import { ModelsUsecases } from '@/usecases/models/models.usecases';
import { CreateModelDto } from '@/infrastructure/dtos/models/create-model.dto';
import { UpdateModelDto } from '@/infrastructure/dtos/models/update-model.dto';
import { ModelDto } from '@/infrastructure/dtos/models/model.dto';
import { ListModelsResponseDto } from '@/infrastructure/dtos/models/list-model-response.dto';
import { DeleteModelResponseDto } from '@/infrastructure/dtos/models/delete-model.dto';
import { ApiOperation, ApiParam, ApiTags, ApiResponse } from '@nestjs/swagger';
import { StartModelSuccessDto } from '@/infrastructure/dtos/models/start-model-success.dto';
import { TransformInterceptor } from '../interceptors/transform.interceptor';
import { CortexUsecases } from '@/usecases/cortex/cortex.usecases';
import { ModelSettingsDto } from '../dtos/models/model-settings.dto';
import { EventName } from '@/domain/telemetry/telemetry.interface';
import { TelemetryUsecases } from '@/usecases/telemetry/telemetry.usecases';
import { CommonResponseDto } from '../dtos/common/common-response.dto';
import { HuggingFaceRepoSibling } from '@/domain/models/huggingface.interface';

@ApiTags('Models')
@Controller('models')
@UseInterceptors(TransformInterceptor)
export class ModelsController {
  constructor(
    private readonly modelsUsecases: ModelsUsecases,
    private readonly cortexUsecases: CortexUsecases,
    private readonly telemetryUsecases: TelemetryUsecases,
  ) {}

  @HttpCode(201)
  @ApiResponse({
    status: 201,
    description: 'The model has been successfully created.',
    type: StartModelSuccessDto,
  })
  @ApiOperation({
    summary: 'Create model',
    description: 'Creates a model `.json` instance file manually.',
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
  @ApiOperation({
    summary: 'Start model',
    description: 'Starts a model operation defined by a model `id`.',
  })
  @ApiParam({
    name: 'modelId',
    required: true,
    description: 'The unique identifier of the model.',
  })
  @Post(':modelId(*)/start')
  startModel(
    @Param('modelId') modelId: string,
    @Body() params: ModelSettingsDto,
  ) {
    return this.cortexUsecases
      .startCortex()
      .then(() => this.modelsUsecases.startModel(modelId, params));
  }

  @HttpCode(200)
  @ApiResponse({
    status: 200,
    description: 'The model has been successfully stopped.',
    type: StartModelSuccessDto,
  })
  @ApiOperation({
    summary: 'Stop model',
    description: 'Stops a model operation defined by a model `id`.',
  })
  @ApiParam({
    name: 'modelId',
    required: true,
    description: 'The unique identifier of the model.',
  })
  @Post(':modelId(*)/stop')
  stopModel(@Param('modelId') modelId: string) {
    return this.modelsUsecases.stopModel(modelId);
  }

  @ApiOperation({
    summary: 'Abort model pull',
    description: 'Abort the model pull operation.',
    parameters: [
      {
        in: 'path',
        name: 'pull_id',
        required: true,
        description: 'The unique identifier of the pull.',
      },
    ],
  })
  @Delete(':pull_id(*)/pull')
  abortPullModel(@Param('pull_id') pullId: string) {
    return this.modelsUsecases.abortDownloadModel(pullId);
  }

  @HttpCode(200)
  @ApiResponse({
    status: 200,
    description: 'Ok',
    type: CommonResponseDto,
  })
  @ApiOperation({
    summary: 'Download a remote model',
    description:
      'Pulls a remote model template from cortex hub or huggingface and downloads it.',
  })
  @ApiParam({
    name: 'modelId',
    required: true,
    description: 'The unique identifier of the model.',
  })
  @ApiParam({
    name: 'fileName',
    required: false,
    description: 'The file name of the model to download.',
  })
  @ApiParam({
    name: 'persistedModelId',
    required: false,
    description: 'The unique identifier of the model in your local storage.',
  })
  @Post(':modelId(*)/pull')
  pullModel(
    @Param('modelId') modelId: string,
    @Body() body?: {
      fileName?: string;
      persistedModelId?: string;
    }
  ) {
    const { fileName, persistedModelId } = body || {};
    this.modelsUsecases
      .pullModel(
        modelId,
        false,
        (files) => {
          return new Promise<HuggingFaceRepoSibling>(
            async (resolve, reject) => {
              const file = files.find(
                (e) =>
                  e.quantization && (!fileName || e.rfilename === fileName),
              );
              if (!file) {
                return reject(new BadRequestException('File not found'));
              }
              return resolve(file);
            },
          );
        },
        persistedModelId,
      )
      .then(() =>
        this.telemetryUsecases.addEventToQueue({
          name: EventName.DOWNLOAD_MODEL,
          modelId,
        }),
      );
    return {
      message: 'Download model started successfully.',
    };
  }

  @HttpCode(200)
  @ApiResponse({
    status: 200,
    description: 'Ok',
    type: ListModelsResponseDto,
  })
  @ApiOperation({
    summary: 'List models',
    description:
      "Lists the currently available models, and provides basic information about each one such as the owner and availability. [Equivalent to OpenAI's list model](https://platform.openai.com/docs/api-reference/models/list).",
  })
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
  @ApiOperation({
    summary: 'Get model',
    description:
      "Retrieves a model instance, providing basic information about the model such as the owner and permissions. [Equivalent to OpenAI's list model](https://platform.openai.com/docs/api-reference/models/retrieve).",
  })
  @ApiParam({
    name: 'id',
    required: true,
    description: 'The unique identifier of the model.',
  })
  @Get(':id(*)')
  findOne(@Param('id') id: string) {
    return this.modelsUsecases.findOne(id);
  }

  @HttpCode(200)
  @ApiResponse({
    status: 200,
    description: 'The model has been successfully updated.',
    type: UpdateModelDto,
  })
  @ApiOperation({
    summary: 'Update model',
    description: "Updates a model instance defined by a model's `id`.",
    parameters: [
      {
        in: 'path',
        name: 'model',
        required: true,
        description: 'The unique identifier of the model.',
      },
    ],
  })
  @Patch(':model(*)')
  async update(
    @Param('model') model: string,
    @Body() updateModelDto: UpdateModelDto,
  ) {
    return this.modelsUsecases.update(model, updateModelDto);
  }

  @ApiResponse({
    status: 200,
    description: 'The model has been successfully deleted.',
    type: DeleteModelResponseDto,
  })
  @ApiOperation({
    summary: 'Delete model',
    description:
      "Deletes a model. [Equivalent to OpenAI's delete model](https://platform.openai.com/docs/api-reference/models/delete).",
  })
  @ApiParam({
    name: 'id',
    required: true,
    description: 'The unique identifier of the model.',
  })
  @Delete(':id(*)')
  remove(@Param('id') id: string) {
    return this.modelsUsecases.remove(id);
  }
}
