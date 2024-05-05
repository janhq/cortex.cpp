import { Body, Controller, HttpCode, Post } from '@nestjs/common';
import { ApiResponse, ApiTags } from '@nestjs/swagger';
import { CortexService } from './cortex.service';
import { StartCortexDto } from './dto/start-cortex.dto';
import { CortexOperationSuccessfullyDto } from './dto/cortex-operation-successfully.dto';

@ApiTags('Cortex')
@Controller('cortex')
export class CortexController {
  constructor(private readonly cortexService: CortexService) {}

  @HttpCode(200)
  @ApiResponse({
    status: 200,
    description: 'Start Cortex successfully',
    type: CortexOperationSuccessfullyDto,
  })
  @Post('start')
  startCortex(@Body() startCortexDto: StartCortexDto) {
    return this.cortexService.startCortex(
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
    return this.cortexService.stopCortex();
  }
}
