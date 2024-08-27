import { TelemetrySource } from '@/domain/telemetry/telemetry.interface';
import { TelemetryUsecases } from '@/usecases/telemetry/telemetry.usecases';
import {
  ArgumentsHost,
  Catch,
  ExceptionFilter,
  HttpException,
  HttpStatus,
} from '@nestjs/common';
import { Response } from 'express';

@Catch()
export class GlobalExceptionFilter implements ExceptionFilter {
  constructor(private readonly telemetryService: TelemetryUsecases) {}
  async catch(exception: HttpException | Error, host: ArgumentsHost) {
    const isHttpException = exception instanceof HttpException;
    const httpStatus = isHttpException
      ? exception.getStatus()
      : HttpStatus.INTERNAL_SERVER_ERROR;

    const message = isHttpException
      ? exception.message
      : 'Internal Server Error';

    const response = host.switchToHttp().getResponse<Response>();

    if (!isHttpException || httpStatus === HttpStatus.INTERNAL_SERVER_ERROR) {
      await this.telemetryService.createCrashReport(
        exception,
        TelemetrySource.CORTEX_SERVER,
      );
      await this.telemetryService.sendCrashReport();
    }
    response.status(httpStatus).json({
      statusCode: httpStatus,
      message,
    });
  }
}
