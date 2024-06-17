import { LlmEngine } from '@/domain/models/model.interface';

export interface ModelStat {
  modelId: string;
  engine?: LlmEngine;
  duration?: string;
  status: string;
  vram?: string;
  ram?: string;
}
