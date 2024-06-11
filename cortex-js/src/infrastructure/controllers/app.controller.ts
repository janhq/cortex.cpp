import {
  DownloadState,
  DownloadStateEvent,
} from '@/domain/models/download.interface';
import { Controller, Sse } from '@nestjs/common';
import { EventEmitter2 } from '@nestjs/event-emitter';
import { Observable, fromEvent, map } from 'rxjs';

@Controller('app')
export class AppController {
  constructor(private readonly eventEmitter: EventEmitter2) {}

  @Sse('download')
  downloadEvent(): Observable<DownloadStateEvent> {
    return fromEvent(this.eventEmitter, 'download.event').pipe(
      map((downloadState: DownloadState[]) => ({ data: downloadState })),
    );
  }
}
