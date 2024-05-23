export const normalizeModelId = (modelId: string): string => {
  return modelId.replace(':', '%3A');
};
