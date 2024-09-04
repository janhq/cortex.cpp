import { isAbsolute } from 'path';

/**
 * Check if a string is a valid URL.
 * @param input - The string to check.
 * @returns True if the string is a valid URL, false otherwise.
 */
export function isValidUrl(input: string | undefined): boolean {
  if (!input) return false;
  try {
    new URL(input);
    return true;
  } catch (e) {
    return false;
  }
}

/**
 * Check if the URL is a lcoal file path
 * @param modelFiles
 * @returns
 */
export const isLocalFile = (path: string): boolean => {
  return !/^(http|https):\/\/[^/]+\/.*/.test(path) && isAbsolute(path);
};
