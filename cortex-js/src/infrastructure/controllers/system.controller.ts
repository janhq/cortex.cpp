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
import { DownloadManagerService } from '@/infrastructure/services/download-manager/download-manager.service';
import { ModelsUsecases } from '@/usecases/models/models.usecases';
import { Controller, Delete, Get, HttpCode, Sse } from '@nestjs/common';
import { EventEmitter2 } from '@nestjs/event-emitter';
import { ApiOperation, ApiResponse, ApiTags } from '@nestjs/swagger';
import {
  Observable,
  catchError,
  combineLatest,
  distinctUntilChanged,
  from,
  fromEvent,
  interval,
  map,
  merge,
  of,
  startWith,
  switchMap,
} from 'rxjs';
import { ResourcesManagerService } from '../services/resources-manager/resources-manager.service';
import { ResourceEvent } from '@/domain/models/resource.interface';
import { CortexUsecases } from '@/usecases/cortex/cortex.usecases';

@ApiTags('System')
@Controller('system')
export class SystemController {
  constructor(
    private readonly downloadManagerService: DownloadManagerService,
    private readonly modelsUsecases: ModelsUsecases,
    private readonly cortexUsecases: CortexUsecases,
    private readonly eventEmitter: EventEmitter2,
    private readonly resourcesManagerService: ResourcesManagerService,
  ) {}

  @ApiOperation({
    summary: 'Stop api server',
    description: 'Stops the Cortex API endpoint server for the detached mode.',
  })
  @Delete()
  async delete() {
    await this.cortexUsecases.stopCortex().catch(() => {});
    process.exit(0);
  }

  @ApiOperation({
    summary: 'Get health status',
    description: "Retrieves the health status of your Cortex's system.",
  })
  @HttpCode(200)
  @ApiResponse({
    status: 200,
    description: 'Ok',
  })
  @Get()
  async get() {
    return 'OK';
  }

  @ApiOperation({
    summary: 'Get download status',
    description: "Retrieves the model's download status.",
  })
  @Sse('events/download')
  downloadEvent(): Observable<DownloadStateEvent> {
    const latestDownloadState$: Observable<DownloadStateEvent> = of({
      data: this.downloadManagerService.getDownloadStates(),
    });
    const downloadEvent$ = fromEvent<DownloadState[]>(
      this.eventEmitter,
      'download.event',
    ).pipe(
      map((downloadState) => ({ data: downloadState })),
      distinctUntilChanged(),
    );

    return merge(latestDownloadState$, downloadEvent$).pipe();
  }

  @ApiOperation({
    summary: 'Get model status',
    description: 'Retrieves all the available model statuses within Cortex.',
  })
  @Sse('events/model')
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

  @ApiOperation({
    summary: 'Get resources status',
    description: 'Retrieves the resources status of the system.',
  })
  @Sse('events/resources')
  resourcesEvent(): Observable<ResourceEvent> {
    const initialData$ = from(
      this.resourcesManagerService.getResourceStatuses(),
    ).pipe(
      map((data) => ({ data: data })),
      catchError((error) => {
        console.error('Error fetching initial resource statuses', error);
        return of(); // Ensure the stream is kept alive even if initial fetch fails
      }),
    );

    const getResourceStatuses$ = interval(2000).pipe(
      switchMap(() => this.resourcesManagerService.getResourceStatuses()),
      map((data) => ({ data: data })),
      catchError((error) => {
        console.error('Error fetching resource statuses', error);
        return of(); // Keep the stream alive on error
      }),
    );

    // Merge the initial data with the interval updates
    return merge(initialData$, getResourceStatuses$);
  }
}
