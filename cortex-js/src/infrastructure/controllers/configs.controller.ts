import {
  Controller,
  Get,
  Post,
  Body,
  Param,
  HttpCode,
  UseInterceptors,
} from '@nestjs/common';
import { ApiOperation, ApiParam, ApiTags, ApiResponse } from '@nestjs/swagger';
import { TransformInterceptor } from '../interceptors/transform.interceptor';
import { ConfigsUsecases } from '@/usecases/configs/configs.usecase';
import { ConfigUpdateDto } from '../dtos/configs/config-update.dto';
import { CommonResponseDto } from '../dtos/common/common-response.dto';

@ApiTags('Configurations')
@Controller('configs')
@UseInterceptors(TransformInterceptor)
export class ConfigsController {
  constructor(private readonly configsUsecases: ConfigsUsecases) {}

  @HttpCode(200)
  @ApiResponse({
    status: 200,
    description: 'Ok',
    type: [Object],
  })
  @ApiOperation({
    summary: 'List configs',
    description:
      'Lists the currently available configs, including the default and user-defined configurations.',
  })
  @Get()
  findAll() {
    return this.configsUsecases.getConfigs();
  }

  @HttpCode(200)
  @ApiResponse({
    status: 200,
    description: 'Ok',
    type: Object,
  })
  @ApiOperation({
    summary: 'Get a config',
    description:
      'Retrieves a config instance, providing basic information about the config.',
  })
  @ApiParam({
    name: 'name',
    required: true,
    description: 'The unique identifier of the config.',
  })
  @Get(':name(*)')
  findOne(@Param('name') name: string) {
    return this.configsUsecases.getGroupConfigs(name);
  }

  @HttpCode(200)
  @ApiResponse({
    status: 200,
    description: 'The config has been successfully updated.',
    type: CommonResponseDto,
  })
  @ApiOperation({
    summary: 'Configure a model',
    description: "Updates a specific configuration setting by its group and key.",
    parameters: [
      {
        in: 'path',
        name: 'model',
        required: true,
        description: 'The unique identifier of the model.',
      },
    ],
  })
  @Post(':name(*)')
  async update(@Param('name') name: string, @Body() configs: ConfigUpdateDto) {
    return this.configsUsecases.saveConfig(configs.key, configs.value, name);
  }
}
