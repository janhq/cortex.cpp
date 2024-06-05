import { ModelArtifact } from '@/domain/models/model.interface';

export const normalizeModelId = (modelId: string): string => {
  return modelId.replace(/[:/]/g, '-');
};

export const isLocalModel = (
  modelFiles?: string[] | ModelArtifact,
): boolean => {
  return (
    !!modelFiles &&
    Array.isArray(modelFiles) &&
    !/^(http|https):\/\/[^/]+\/.*/.test(modelFiles[0])
  );
};
