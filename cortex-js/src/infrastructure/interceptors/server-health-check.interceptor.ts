import { CortexUsecases } from '@/usecases/cortex/cortex.usecases';
import {
  CallHandler,
  ExecutionContext,
  Injectable,
  NestInterceptor,
} from '@nestjs/common';
import { SubCommand } from 'nest-commander';
import { from, Observable } from 'rxjs';
import { mergeMap, tap } from 'rxjs/operators';

@Injectable()
export class ServerHealthCheckInterceptor implements NestInterceptor {
  constructor(private readonly cortexUsecases: CortexUsecases) {}

  intercept(
    _context: ExecutionContext,
    next: CallHandler,
  ): Observable<typeof SubCommand> {
    console.log('Checking API server health...');
    return from(this.cortexUsecases.isAPIServerOnline()).pipe(
      mergeMap((result) => {
        console.log(result);

        return next.handle().pipe(tap(() => null));
      }),
    );
  }
}
