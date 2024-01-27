import { describe, test } from "@jest/globals";

import fs from "node:fs";
import os from "node:os";
import path from "node:path";

import download from "download";

import { Duplex } from "node:stream";
import {
  stopModel,
  runModel,
  loadLLMModel,
  validateModelStatus,
  chatCompletion,
} from "../src";

// FIXME: Shorthand only possible for es6 targets and up
//import * as model from './model.json' assert {type: 'json'}

// Test assets dir
const TEST_ASSETS_PATH = path.join(__dirname, "test_assets");
const MODEL_CONFIG_PATH = path.join(TEST_ASSETS_PATH, "model.json");

// Get model config
const getModelConfigHook = (callback: (modelCfg: any) => void) => () => {
  const modelJson = fs.readFileSync(MODEL_CONFIG_PATH, { encoding: "utf8" });
  const modelCfg = JSON.parse(modelJson);
  callback(modelCfg);
};

// Report download progress
const createProgressReporter =
  (name: string) => (stream: Promise<Buffer> & Duplex) =>
    stream
      .on(
        "downloadProgress",
        (progress: { transferred: any; total: any; percent: number }) => {
          // Print and update progress on a single line of terminal
          process.stdout.write(
            `\r\x1b[K[${name}] ${progress.transferred}/${progress.total} ${Math.floor(progress.percent * 100)}%...`,
          );
        },
      )
      .on("end", () => {
        // Jump to new line to log next message
        process.stdout.write(`${os.EOL}[${name}] Finished downloading!`);
      });

// Download model file
const downloadModelHook = (modelCfg: any, targetDir: string) => async () => {
  const fileName = modelCfg.source_url.split("/")?.pop() ?? "model.gguf";
  // Check if there is a downloaded model at TEST_ASSETS_PATH
  const downloadedModelFile = fs
    .readdirSync(TEST_ASSETS_PATH)
    .find((fname) => fname.match(/.*\.gguf/gi));
  if (downloadedModelFile) {
    const downloadedModelPath = path.join(
      TEST_ASSETS_PATH,
      downloadedModelFile,
    );
    // Copy model files to targetDir and return
    fs.cpSync(downloadedModelPath, path.join(targetDir, fileName));
    console.log(
      `Reuse cached model ${modelCfg.name} from path ${downloadedModelPath} => ${targetDir}`,
    );
    return;
  }
  const progressReporter = createProgressReporter(modelCfg.name);
  await progressReporter(
    download(modelCfg.source_url, targetDir, {
      filename: fileName,
      strip: 1,
      extract: true,
    }),
  );
  console.log(
    `Downloaded model ${modelCfg.name} at path ${path.join(targetDir, fileName)}`,
  );
};

// Cleanup tmp directory that is used during tests
const cleanupTargetDirHook = (targetDir: string) => () => {
  fs.rmSync(targetDir, {
    recursive: true, // Remove whole directory
    maxRetries: 3, // Retry 3 times on error
    retryDelay: 250, // Back-off with 250ms delay
  });
};

/**
 * Sleep for the specified milliseconds
 * @param {number} ms milliseconds to sleep for
 * @returns {Promise<NodeJS.Timeout>}
 */
const sleep = async (ms: number): Promise<NodeJS.Timeout> =>
  Promise.resolve().then(() => setTimeout(() => void 0, ms));

/**
 * Basic test suite
 */
describe("Manage nitro process", () => {
  /// BEGIN SUITE CONFIG
  const modelFullPath = fs.mkdtempSync(
    path.join(os.tmpdir(), "nitro-node-test"),
  );
  let modelCfg: any = {};

  // Setup steps before running the suite
  const setupHooks = [
    // Get model config from json
    getModelConfigHook((cfg) => Object.assign(modelCfg, cfg)),
    // Download model before starting tests
    downloadModelHook(modelCfg, modelFullPath),
  ];
  // Teardown steps after running the suite
  const teardownHooks = [
    // Stop nitro after running, regardless of error or not
    () => stopModel(),
    // On teardown, cleanup tmp directory that was created earlier
    cleanupTargetDirHook(modelFullPath),
  ];
  /// END SUITE CONFIG

  /// BEGIN HOOKS REGISTERING
  beforeAll(
    // Run all the hooks sequentially
    async () => setupHooks.reduce((p, fn) => p.then(fn), Promise.resolve()),
    // Set timeout for tests to wait for downloading model before run
    10 * 60 * 1000,
  );
  afterAll(
    // Run all the hooks sequentially
    async () => teardownHooks.reduce((p, fn) => p.then(fn), Promise.resolve()),
    // Set timeout for cleaning up
    10 * 60 * 1000,
  );
  /// END HOOKS REGISTERING

  /// BEGIN TESTS
  test(
    "start/stop nitro process normally",
    async () => {
      // Start nitro
      await runModel({
        modelFullPath,
        promptTemplate: modelCfg.settings.prompt_template,
      });
      // Wait 5s for nitro to start
      await sleep(5 * 1000);
      // Stop nitro
      await stopModel();
    },
    // Set timeout to 30 seconds
    30 * 1000,
  );
  test(
    "chat completion",
    async () => {
      // Start nitro
      await runModel({
        modelFullPath,
        promptTemplate: modelCfg.settings.prompt_template,
      });
      // Wait 5s for nitro to start
      await sleep(5 * 1000);
      // Load LLM model
      await loadLLMModel({
        llama_model_path: modelFullPath,
        ctx_len: modelCfg.settings.ctx_len,
        ngl: modelCfg.settings.ngl,
        embedding: false,
      });
      // Validate model status
      await validateModelStatus();
      // Arrays of all the chunked response
      let streamedContent: string[] = [];
      // Run chat completion with stream
      const response = await chatCompletion(
        {
          messages: [
            { content: "Hello there", role: "assistant" },
            { content: "Write a long and sad story for me", role: "user" },
          ],
          model: "gpt-3.5-turbo",
          max_tokens: 50,
          stop: ["hello"],
          frequency_penalty: 0,
          presence_penalty: 0,
          temperature: 0.1,
        },
        new WritableStream({
          write(chunk: string) {
            if (chunk.trim() == "data: [DONE]") {
              return;
            }
            return new Promise((resolve) => {
              streamedContent.push(chunk.slice("data:".length).trim());
              resolve();
            });
          },
          //close() {},
          //abort(_err) {}
        }),
      );
      // Remove the [DONE] message
      streamedContent.pop();
      // Parse json
      streamedContent = streamedContent.map((str) => JSON.parse(str));
      // Show the streamed content
      console.log(
        `[Streamed response] ${JSON.stringify(streamedContent, null, 2)}`,
      );

      // The response body is unusable if consumed by out stream
      await expect(response.text).rejects.toThrow();
      await expect(response.json).rejects.toThrow();
      // Response body should be used already
      expect(response.bodyUsed).toBeTruthy();
      // There should be multiple chunks of json data
      expect(streamedContent.length).toBeGreaterThan(0);
      // Stop nitro
      await stopModel();
    },
    // Set timeout to 1 minutes
    1 * 60 * 1000,
  );
  /// END TESTS
});
