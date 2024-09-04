import { HttpException, HttpStatus } from '@nestjs/common';

export class ModelSettingNotFoundException extends HttpException {
  constructor(engine: string) {
    super(`Model setting for ${engine} not found!`, HttpStatus.NOT_FOUND);
  }
}
