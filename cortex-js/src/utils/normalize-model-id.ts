import { ModelArtifact } from '@/domain/models/model.interface';
import { getGpuInfo } from './cuda';
import {
  Engines,
  RemoteEngines,
} from '@/infrastructure/commanders/types/engine.interface';

export const normalizeModelId = (modelId: string): string => {
  return modelId.replace(':main', '').replace(/[:/]/g, '-');
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

export const isRemoteEngine = (engine: string): boolean => {
  return RemoteEngines.includes(engine as Engines);
};

/**
 * Parse the model hub engine branch
 * @param branch
 * @returns
 */
export const parseModelHubEngineBranch = async (
  branch: string,
): Promise<string> => {
  if (branch.includes('tensorrt')) {
    let engineBranch = branch;
    const platform = process.platform == 'win32' ? 'windows' : 'linux';
    if (!engineBranch.includes(platform)) {
      engineBranch += `-${platform}`;
    }

    const gpus = await getGpuInfo();
    if (gpus[0]?.arch && !engineBranch.includes(gpus[0].arch)) {
      engineBranch += `-${gpus[0].arch}`;
    }
    return engineBranch;
  }
  return branch;
};
