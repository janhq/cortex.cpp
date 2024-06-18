import { Controller, HttpCode, Get } from '@nestjs/common';
import { ApiOperation, ApiTags, ApiResponse } from '@nestjs/swagger';

@ApiTags('Status')
@Controller('health')
export class StatusController {
  constructor() {}

  @ApiOperation({
    summary: "Get health status",
    description: "Retrieves the health status of the Cortex's API endpoint server.",
  })
  @HttpCode(200)
  @ApiResponse({
    status: 200,
    description: 'Ok',
  })
  @Get()
  async get() {
    return 'OK';
  }
}
