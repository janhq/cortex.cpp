import {
  Controller,
  Get,
  Param,
  HttpCode,
  UseInterceptors,
  Post,
  Body,
  Patch,
  Res,
} from '@nestjs/common';
import { ApiOperation, ApiParam, ApiTags, ApiResponse } from '@nestjs/swagger';
import { Response } from 'express';
import { TransformInterceptor } from '../interceptors/transform.interceptor';
import { EnginesUsecases } from '@/usecases/engines/engines.usecase';
import { EngineDto, InitEngineDto } from '../dtos/engines/engines.dto';
import { CommonResponseDto } from '../dtos/common/common-response.dto';

@ApiTags('Engines')
@Controller('engines')
@UseInterceptors(TransformInterceptor)
export class EnginesController {
  constructor(private readonly enginesUsecases: EnginesUsecases) {}

  @HttpCode(200)
  @ApiResponse({
    status: 200,
    description: 'Ok',
    type: [EngineDto],
  })
  @ApiOperation({
    summary: 'List available engines',
    description:
      'Lists the currently available engines, including local and remote engines.',
  })
  @Get()
  findAll() {
    return this.enginesUsecases.getEngines();
  }

  @HttpCode(200)
  @ApiResponse({
    status: 200,
    description: 'Ok',
    type: EngineDto,
  })
  @ApiOperation({
    summary: 'Get an engine',
    description:
      'Retrieves an engine instance, providing basic information about the engine.',
  })
  @ApiParam({
    name: 'name',
    required: true,
    description: 'The unique identifier of the engine.',
  })
  @Get(':name(*)')
  findOne(@Param('name') name: string) {
    return this.enginesUsecases.getEngine(name);
  }

  @HttpCode(200)
  @ApiResponse({
    status: 200,
    description: 'Ok',
    type: CommonResponseDto,
  })
  @ApiOperation({
    summary: 'Initialize an engine',
    description:
      'Initializes an engine instance with the given name. It will download the engine if it is not available locally.',
  })
  @ApiParam({
    name: 'name',
    required: true,
    description: 'The unique identifier of the engine.',
  })
  @Post(':name(*)/init')
  initialize(
    @Param('name') name: string,
    @Body() body: InitEngineDto | undefined,
    @Res() res: Response,
  ) {
    try {
      this.enginesUsecases.installEngine(body, name, true);
      res.json({
        message: 'Engine initialization started successfully.',
      });
    } catch (error) {
      res.status(400).send(error.message);
    }
  }

  @HttpCode(200)
  @ApiResponse({
    status: 200,
    description: 'Ok',
    type: CommonResponseDto,
  })
  @ApiOperation({
    summary: 'Update an engine',
    description: 'Updates the engine with configurations.',
  })
  @ApiParam({
    name: 'name',
    required: true,
    description: 'The unique identifier of the engine.',
  })
  @Patch(':name(*)')
  update(@Param('name') name: string, @Body() configs?: any | undefined) {
    console.log('configs', configs);
    return this.enginesUsecases.updateConfigs(
      configs.config,
      configs.value,
      name,
    );
  }
}
