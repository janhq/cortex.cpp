import { HttpException, HttpStatus } from '@nestjs/common';

export class ModelNotFoundException extends HttpException {
  constructor(modelId: string) {
    super(`Model ${modelId} not found!`, HttpStatus.NOT_FOUND);
  }
}
