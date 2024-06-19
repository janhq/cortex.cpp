import { TelemetrySource } from '@/domain/telemetry/telemetry.interface';
import { ContextService } from '@/util/context.service';
import { Injectable, NestMiddleware, Logger } from '@nestjs/common';

import { Request, Response, NextFunction } from 'express';

@Injectable()
export class AppLoggerMiddleware implements NestMiddleware {
  constructor(private readonly contextService: ContextService) {}
  private logger = new Logger('HTTP');

  use(req: Request, res: Response, next: NextFunction): void {
    const userAgent = req.get('user-agent') ?? '';
    const { ip, method, path: url, originalUrl } = req;
    //Setting the x-correlation-id
    const correlationHeader = req.header('x-correlation-id') ?? '';
    req.headers['x-correlation-id'] = correlationHeader;
    res.set('X-Correlation-Id', correlationHeader);
    res.on('close', () => {
      const { statusCode } = res;
      const contentLength = res.get('content-length');
      this.logger.log(
        JSON.stringify({
          method: method,
          path: originalUrl ?? url,
          statusCode: statusCode,
          ip: ip,
          content_length: contentLength,
          user_agent: userAgent,
          x_correlation_id: req.headers['x-correlation-id'],
        }),
      );
    });
    this.contextService.init(() => {
      this.contextService.set('endpoint', originalUrl ?? url);
      this.contextService.set('source', TelemetrySource.CORTEX_SERVER);
      next();
    });
  }
}
