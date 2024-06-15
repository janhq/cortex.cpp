import { Controller, HttpCode, Get } from '@nestjs/common';
import { ApiOperation, ApiTags, ApiResponse } from '@nestjs/swagger';

@ApiTags('Status')
@Controller('health')
export class StatusController {
  constructor() {}

  @ApiOperation({
    summary: 'Health check',
    description: 'Health check endpoint.',
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
