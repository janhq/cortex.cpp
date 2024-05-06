import { Body, Controller, HttpCode, Post } from '@nestjs/common';
import { ApiResponse, ApiTags } from '@nestjs/swagger';
import { StartCortexDto } from '../dtos/cortex/start-cortex.dto';
import { CortexOperationSuccessfullyDto } from '../dtos/cortex/cortex-operation-successfully.dto';
import { CortexUsecases } from 'src/usecases/cortex/cortex.usecases';

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
  @Post('start')
  startCortex(@Body() startCortexDto: StartCortexDto) {
    return this.cortexUsecases.startCortex(
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
  @Post('stop')
  stopCortex() {
    return this.cortexUsecases.stopCortex();
  }
}
