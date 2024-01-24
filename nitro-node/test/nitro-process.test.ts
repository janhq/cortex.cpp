import { jest, describe, test } from '@jest/globals'

import fs from 'node:fs'
import os from 'node:os'
import path from 'node:path'

import download from 'download'

import { default as nitro } from '../src/index'
import { Duplex } from 'node:stream'
const { runModel, stopModel } = nitro

// FIXME: Shorthand only possible for es6 targets and up
//import * as model from './model.json' assert {type: 'json'}

// Get model config
const getModelConfigHook = (callback: (modelCfg: any) => void) => () => {
  const modelJson = fs.readFileSync(path.join(__dirname, 'model.json'), {
    encoding: 'utf8',
  })
  const modelCfg = JSON.parse(modelJson)
  callback(modelCfg)
}

// Report download progress
const createProgressReporter = (name: string) => (stream: Promise<Buffer> & Duplex) => stream.on(
  'downloadProgress',
  (progress: { transferred: any; total: any; percent: number }) => {
    // Print and update progress on a single line of terminal
    process.stdout.write(`\r[${name}] ${progress.transferred}/${progress.total} ${Math.floor(progress.percent * 100)}%...`);
  }).on('end', () => {
    // Jump to new line to log next message
    process.stdout.write(`\n[${name}] Finished downloading!`);
  })

// Download model file
const downloadModelHook = (modelCfg: any, targetDir: string) => async () => {
  const fileName = modelCfg.source_url.split('/')?.pop() ?? 'model.gguf'
  const progressReporter = createProgressReporter(modelCfg.name)
  await progressReporter(
    download(
      modelCfg.source_url,
      targetDir,
      {
        filename: fileName,
        strip: 1,
        extract: true,
      },
    )
  )
  console.log(`Downloaded model ${modelCfg.name} at path ${path.join(targetDir, fileName)}`)
}

// Cleanup tmp directory that is used during tests
const cleanupTargetDirHook = (targetDir: string) => () => {
  fs.rmSync(targetDir, {
    recursive: true, // Remove whole directory
    maxRetries: 3, // Retry 3 times on error
    retryDelay: 250, // Back-off with 250ms delay
  })
}

/**
 * Sleep for the specified milliseconds
 * @param {number} ms milliseconds to sleep for
 * @returns {Promise<NodeJS.Timeout>}
 */
const sleep = async (ms: number): Promise<NodeJS.Timeout> => Promise.resolve().then(() => setTimeout(() => void (0), ms))

/**
  * Basic test suite
  */
describe('Manage nitro process', () => {
  /// BEGIN SUITE CONFIG
  const modelFullPath = fs.mkdtempSync(path.join(os.tmpdir(), 'nitro-node-test'));
  let modelCfg: any = {}

  // Setup steps before running the suite
  const setupHooks = [
    // Get model config from json
    getModelConfigHook((cfg) => Object.assign(modelCfg, cfg)),
    // Download model before starting tests
    downloadModelHook(modelCfg, modelFullPath),
  ]
  // Teardown steps after running the suite
  const teardownHooks = [
    // On teardown, cleanup tmp directory that was created earlier
    cleanupTargetDirHook(modelFullPath),
  ]
  /// END SUITE CONFIG

  /// BEGIN HOOKS REGISTERING
  beforeAll(
    // Run all the hooks sequentially
    async () => setupHooks.reduce((p, fn) => p.then(fn), Promise.resolve()),
    // Set timeout for tests to wait for downloading model before run
    10 * 60 * 1000,
  )
  afterAll(
    // Run all the hooks sequentially
    async () => teardownHooks.reduce((p, fn) => p.then(fn), Promise.resolve()),
    // Set timeout for cleaning up
    10 * 60 * 1000,
  )
  /// END HOOKS REGISTERING

  /// BEGIN TESTS
  test('start/stop nitro process normally',
    async () => {
      // Start nitro
      await runModel({
        modelFullPath,
        promptTemplate: modelCfg.settings.prompt_template,
      })
      // Wait 5s for nitro to start
      await sleep(5 * 1000)
      // Stop nitro
      await stopModel()
    },
    // Set default timeout to 1 minutes
    1 * 60 * 1000,
  )
  /// END TESTS
})
