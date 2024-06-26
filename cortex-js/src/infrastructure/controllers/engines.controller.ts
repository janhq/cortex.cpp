import {
  Controller,
  Get,
  Param,
  HttpCode,
  UseInterceptors,
} from '@nestjs/common';
import { ApiOperation, ApiParam, ApiTags, ApiResponse } from '@nestjs/swagger';
import { TransformInterceptor } from '../interceptors/transform.interceptor';
import { EnginesUsecases } from '@/usecases/engines/engines.usecase';
import { EngineDto } from '../dtos/engines/engines.dto';

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
      'Lists the currently available engines, including local and remote engines',
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
      'Retrieves an engine instance, providing basic information about the engine',
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
}
