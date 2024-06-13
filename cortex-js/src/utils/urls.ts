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
