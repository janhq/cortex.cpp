import {
  DownloadState,
  DownloadStateEvent,
} from '@/domain/models/download.interface';
import {
  EmptyModelEvent,
  ModelEvent,
  ModelId,
  ModelStatus,
  ModelStatusAndEvent,
} from '@/domain/models/model.event';
import { DownloadManagerService } from '@/download-manager/download-manager.service';
import { ModelsUsecases } from '@/usecases/models/models.usecases';
import { Controller, Sse } from '@nestjs/common';
import { EventEmitter2 } from '@nestjs/event-emitter';
import { ApiOperation, ApiTags } from '@nestjs/swagger';
import {
  Observable,
  combineLatest,
  fromEvent,
  map,
  merge,
  of,
  startWith,
  throttleTime,
} from 'rxjs';

@ApiTags('Events')
@Controller('events')
export class EventsController {
  constructor(
    private readonly downloadManagerService: DownloadManagerService,
    private readonly modelsUsecases: ModelsUsecases,
    private readonly eventEmitter: EventEmitter2,
  ) {}

  @ApiOperation({
    summary: 'Get download status',
    description: "Retrieves the model's download status.",
  })
  @Sse('download')
  downloadEvent(): Observable<DownloadStateEvent> {
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

  @ApiOperation({
    summary: 'Get model status',
    description: 'Retrieves all the available model statuses within Cortex.',
  })
  @Sse('model')
  modelEvent(): Observable<ModelStatusAndEvent> {
    const latestModelStatus$: Observable<Record<ModelId, ModelStatus>> = of(
      this.modelsUsecases.getModelStatuses(),
    );

    const modelEvent$ = fromEvent<ModelEvent>(
      this.eventEmitter,
      'model.event',
    ).pipe(startWith(EmptyModelEvent));

    return combineLatest([latestModelStatus$, modelEvent$]).pipe(
      map(([status, event]) => ({ data: { status, event } })),
    );
  }
}
