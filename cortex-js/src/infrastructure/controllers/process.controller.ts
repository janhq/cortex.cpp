import { Controller, Delete } from '@nestjs/common';
import { ApiOperation, ApiTags } from '@nestjs/swagger';

@ApiTags('Processes')
@Controller('process')
export class ProcessController {
  constructor() {}

  @ApiOperation({
    summary: 'Terminate service',
    description: 'Terminates the Cortex API endpoint server for the detached mode.',
  })
  @Delete()
  async delete() {
    process.exit(0);
  }
}
