import { HttpException, HttpStatus } from '@nestjs/common';

export class InvalidApiUrlException extends HttpException {
  constructor(modelId: string, apiUrl: string | undefined) {
    super(
      `Remote model ${modelId} has configured api url ${apiUrl} is not valid!`,
      HttpStatus.INTERNAL_SERVER_ERROR,
    );
  }
}
