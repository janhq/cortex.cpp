export type ModelId = string;

const ModelLoadingEvents = [
  'starting',
  'stopping',
  'started',
  'stopped',
  'starting-failed',
  'stopping-failed',
] as const;
export type ModelLoadingEvent = (typeof ModelLoadingEvents)[number];

const AllModelStates = ['starting', 'stopping', 'started'] as const;
export type ModelState = (typeof AllModelStates)[number];

export interface ModelStatus {
  model: ModelId;
  status: ModelState;
  metadata: Record<string, unknown>;
}

export interface ModelEvent {
  model: ModelId;
  event: ModelLoadingEvent;
  metadata: Record<string, unknown>;
}

export const EmptyModelEvent = {};

export interface ModelStatusAndEvent {
  data: {
    status: Record<ModelId, ModelStatus>;
    event: ModelEvent | typeof EmptyModelEvent;
  };
}
