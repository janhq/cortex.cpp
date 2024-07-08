import { ModelArtifact } from '@/domain/models/model.interface';
import { existsSync } from 'fs';
import { basename, join } from 'path';

/**
 * Path to the root of the application.
 */
export const appPath = join(__dirname, '../../');

/**
 * Check if a file exists in any of the given paths.
 * @param file
 * @param paths
 * @returns
 */
export const checkFileExistenceInPaths = (
  file: string,
  paths: string[],
): boolean => {
  return paths.some((p) => existsSync(join(p, file)));
};

/**
 * Get the model file name from the given files.
 * @param files
 * @returns
 */
export const llamaModelFile = (
  files: string[] | ModelArtifact | ModelArtifact[],
) => {
  let artifact: any = files;
  // Legacy model.yml
  if (Array.isArray(files)) {
    artifact = files[0];
  }
  const path =
    'llama_model_path' in artifact
      ? (artifact as ModelArtifact).llama_model_path ?? ''
      : 'model_path' in files
        ? (artifact as ModelArtifact).model_path ?? ''
        : (artifact as string[])[0];
  return basename(path);
};
