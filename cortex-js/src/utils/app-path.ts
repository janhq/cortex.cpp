import { existsSync } from 'fs';
import { join } from 'path';

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
