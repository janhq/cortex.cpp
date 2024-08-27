import { HttpException, HttpStatus } from '@nestjs/common';

export class DuplicateAssistantException extends HttpException {
  constructor(assistantId: string) {
    super(
      `Assistant with the id ${assistantId} is already exists.`,
      HttpStatus.CONFLICT,
    );
  }
}
