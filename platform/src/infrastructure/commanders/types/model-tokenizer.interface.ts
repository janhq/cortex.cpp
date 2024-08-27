export interface ModelMetadata {
  contextLength: number;
  ngl: number;
  stopWord?: string;
  promptTemplate: string;
  version: number;
  name?: string;
}
