import {
  DownloadState,
  DownloadStateEvent,
} from '@/domain/models/download.interface';
import { DownloadManagerService } from '@/download-manager/download-manager.service';
import { Controller, Sse } from '@nestjs/common';
import { EventEmitter2 } from '@nestjs/event-emitter';
import { Observable, fromEvent, map, merge, of, throttleTime } from 'rxjs';

@Controller('events')
export class EventsController {
  constructor(
    private readonly downloadManagerService: DownloadManagerService,
    private readonly eventEmitter: EventEmitter2,
  ) {}

  @Sse('download')
  downloadEvent(): Observable<DownloadStateEvent> {
    // Welcome message Observable
    const latestDownloadState$: Observable<DownloadStateEvent> = of({
      data: this.downloadManagerService.getDownloadStates(),
    });

    const downloadAbortEvent$ = fromEvent<DownloadState[]>(
      this.eventEmitter,
      'download.event.aborted',
    ).pipe(map((downloadState) => ({ data: downloadState })));

    const downloadEvent$ = fromEvent<DownloadState[]>(
      this.eventEmitter,
      'download.event',
    ).pipe(
      map((downloadState) => ({ data: downloadState })),
      throttleTime(1000),
    );

    return merge(
      latestDownloadState$,
      downloadEvent$,
      downloadAbortEvent$,
    ).pipe();
  }
}
