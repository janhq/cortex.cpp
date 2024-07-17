/**
 * Represents a base extension.
 * This class should be extended by any class that represents an extension.
 */
export abstract class Extension {
  /** @type {string} Name of the extension. */
  name: string;

  /** @type {string} Product Name of the extension. */
  productName?: string;

  /** @type {string} The URL of the extension to load. */
  url?: string;

  /** @type {boolean} Whether the extension is activated or not. */
  active?: boolean;

  /** @type {string} Extension's description. */
  description?: string;

  /** @type {string} Extension's version. */
  version?: string;

  /** @type {boolean} Whether the extension is initialized or not. */
  initalized: boolean;

  /**
   * Called when the extension is loaded.
   * Any initialization logic for the extension should be put here.
   */
  onLoad(): void {}

  /**
   * Called when the extension is unloaded.
   * Any cleanup logic for the extension should be put here.
   */
  onUnload(): void {}
}
