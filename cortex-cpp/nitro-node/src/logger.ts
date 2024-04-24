import os from "node:os";
import { NitroLogger } from "./types";

// The logger to use, default to stdout
export let log: NitroLogger = (message, ..._) =>
  process.stdout.write(message + os.EOL);

/**
 * Set logger before running nitro
 * @param {NitroLogger} logger The logger to use
 */
export async function setLogger(logger: NitroLogger): Promise<void> {
  log = logger;
}
