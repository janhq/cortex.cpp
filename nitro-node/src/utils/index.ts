import fs from "node:fs";
import { log } from "../logger";

/**
 * Read the magic bytes from a file and check if they match the provided magic bytes
 */
export async function checkMagicBytes(
  filePath: string,
  magicBytes: string,
): Promise<boolean> {
  const desired = Buffer.from(magicBytes);
  const nBytes = desired.byteLength;
  const chunks = [];
  for await (let chunk of fs.createReadStream(filePath, {
    start: 0,
    end: nBytes - 1,
  })) {
    chunks.push(chunk);
  }
  const actual = Buffer.concat(chunks);
  log(
    `Comparing file's magic bytes <${actual.toString()}> and desired <${desired.toString()}>`,
  );
  return Buffer.compare(actual, desired) === 0;
}
