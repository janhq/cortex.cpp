import { Body, Controller, HttpCode, Post } from '@nestjs/common';
import { ApiResponse, ApiTags, ApiOperation } from '@nestjs/swagger';
import { StartCortexDto } from '@/infrastructure/dtos/cortex/start-cortex.dto';
import { CortexOperationSuccessfullyDto } from '../dtos/cortex/cortex-operation-successfully.dto';
import { CortexUsecases } from '@/usecases/cortex/cortex.usecases';

@ApiTags('Cortex')
@Controller('cortex')
export class CortexController {
  constructor(private readonly cortexUsecases: CortexUsecases) {}

  @HttpCode(200)
  @ApiResponse({
    status: 200,
    description: 'Start Cortex successfully',
    type: CortexOperationSuccessfullyDto,
  })
  @ApiOperation({
    summary: 'Start cortex',
    description: 'Starts the cortex operation.',
  })
  @Post('start')
  startCortex(@Body() startCortexDto: StartCortexDto) {
    return this.cortexUsecases.startCortex(
      false,
      startCortexDto.host,
      startCortexDto.port,
    );
  }

  @HttpCode(200)
  @ApiResponse({
    status: 200,
    description: 'Stop Cortex successfully',
    type: CortexOperationSuccessfullyDto,
  })
  @ApiOperation({
    summary: 'Stop cortex',
    description: 'Stops the cortex operation.',
  })
  @Post('stop')
  stopCortex() {
    return this.cortexUsecases.stopCortex();
  }
}
