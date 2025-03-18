---
name: QA Checklist
about: QA Checklist
title: 'QA: [VERSION]'
labels: 'type: QA checklist'
assignees: ''
---
**QA details:**

Version: `v1.0.x-xxx`

OS (select one)
- [ ] Windows 11 (online & offline)
- [ ] Ubuntu 24, 22 (online & offline)
- [ ] Mac Silicon OS 14/15 (online & offline)
- [ ] Mac Intel (online & offline)

--------

# 1. Manual QA (CLI)
## Installation
- [ ]  it should install with local installer (default; no internet required during installation, all dependencies bundled)
- [ ]  it should install with network installer
- [ ] it should install 2 binaries (cortex and cortex-server) [mac: binaries in `/usr/local/bin`]
- [ ]  it should install with correct folder permissions
- [ ]  it should install with folders: /engines /logs (no /models folder until model pull)
- [ ] It should install with Docker image https://cortex.so/docs/installation/docker/

## Data/Folder structures
- [ ] cortex.so models are stored in `cortex.so/model_name/variants/`, with .gguf and model.yml file
- [ ] huggingface models are stored `huggingface.co/author/model_name` with .gguf and model.yml file
- [ ] downloaded models are saved in cortex.db with the right fields: `model`, `author_repo_id`, `branch_name`, `path_to_model_yaml` (view via SQL)

## Cortex Update
- [ ] cortex -v should check output current version and check for updates
- [ ] cortex update replaces the app, installer, uninstaller and binary file (without installing cortex.llamacpp)
- [ ]  `cortex update` should update from ~3-5 versions ago to latest (+3 to 5 bump)
- [ ]  `cortex update` should update from the previous version to latest (+1 bump)
- [ ] `cortex update -v 1.x.x-xxx` should update from the previous version to specified version
- [ ] `cortex update` should update from previous stable version to latest
- [ ] it should gracefully update when server is actively running

## Overall / App Shell
- [ ] cortex returns helpful text in a timely* way (< 5s)
- [ ] `cortex` or `cortex -h` displays help commands
- [ ] CLI commands should start the API server, if not running [except 
- [ ] it should correctly log to cortex-cli.log and cortex.log
- [ ] There should be no stdout from inactive shell session

## Engines
- [ ] llama.cpp should be installed by default
- [ ] it should run gguf models on llamacpp
- [ ] it should list engines
- [ ] it should get engines
- [ ] it should install engines (latest version if not specified)
- [ ] it should install engines (with specified variant and version)
- [ ] it should get default engine
- [ ] it should set default engine (with specified variant/version)
- [ ] it should load engine
- [ ] it should unload engine
- [ ] it should update engine (to latest version)
- [ ] it should update engine (to specified version)
- [ ] it should uninstall engines
- [ ]  it should gracefully continue engine installation if interrupted halfway (partial download)
- [ ]  it should gracefully handle when users try to CRUD incompatible engines (No variant found for xxx)
- [ ]  it should run trtllm models on trt-llm [WIP,  not tested]
- [ ] it shoud handle engine variants [WIP, not tested]
- [ ]  it should update engines versions [WIP, not tested]

## Server
- [ ] `cortex start` should start server and output localhost URL & port number
- [ ] users can access API Swagger documentation page  at localhost URL & port number
- [ ] `cortex start` can be configured with parameters (port, [logLevel [WIP]](https://github.com/menloresearch/cortex.cpp/pull/1636)) https://cortex.so/docs/cli/start/
- [ ]  it should correctly log to cortex logs (logs/cortex.log, logs/cortex-cli.log)
- [ ]  `cortex ps` should return server status and running models (or no model loaded)
- [ ]  `cortex stop` should stop server

## Model Pulling
- [ ] Pulling a model should pull .gguf and model.yml file
- [ ] Model download progress should appear as download bars for each file
- [ ] Model download progress should be accurate (%, total time, download size, speed)
### cortex.so
- [ ]  it should pull by built in model_ID
- [ ] pull by model_ID should recommend default variant at the top (set in HF model.yml)
- [ ]  it should pull by built-in model_id:variant
### huggingface.co
- [ ]  it should pull by HF repo/model ID
- [ ]  it should pull by full HF url (ending in .gguf)
### Interrupted Download
- [ ] it should allow user to interrupt / stop download 
- [ ] pulling again after interruption should accurately calculates remainder of model file size neeed to be downloaded (`Found unfinished download! Additional XGB needs to be downloaded`)
- [ ]  it should allow to continue downloading the remainder after interruption

## Model Management
- [ ]  it should list downloaded models
- [ ] it should get a local model
- [ ]  it should update model parameters in model.yaml
- [ ] it should delete a model
- [ ]  it should import models with model_id and model_path

## Model Running
- [ ] `cortex run <cortexso model>` - if no local models detected, shows `pull` model menu
- [ ] `cortex run` - if local model detected, runs the local model
- [ ]  `cortex run` - if multiple local models detected, shows list of local models (from multiple model sources eg cortexso, HF authors) for users to select (via regex search)
- [ ] `cortex run <invalid model id>` should return gracefully `Model not found!`
- [ ]  run should autostart server
- [ ] `cortex run <model>` starts interactive chat (by default)
- [ ] `cortex run <model> -d` runs in detached mode
- [ ] `cortex models start <model>`  
- [ ] terminate StdIn or `exit()` should exit interactive chat

## Hardware Detection / Acceleration [WIP, no need to QA]
- [ ]  it should auto offload max ngl
- [ ]  it should correctly detect available GPUs
- [ ]  it should gracefully detect missing dependencies/drivers
CPU Extension (e.g. AVX-2, noAVX, AVX-512)
GPU Acceleration (e.g. CUDA11, CUDA12, Vulkan, sycl, etc)

## Uninstallation / Reinstallation
- [ ]  it should uninstall 2 binaries (cortex and cortex-server)
- [ ]  it should uninstall with 2 options to delete or not delete data folder 
- [ ]  it should gracefully uninstall when server is still running
- [ ] uninstalling should not leave any dangling files
- [ ] uninstalling should not leave any dangling processes
- [ ]  it should reinstall without having conflict issues with existing cortex data folders

--
# 2. API QA

## Checklist for each endpoint
- [ ] Upon `cortex start`, API page is displayed at localhost:port endpoint
- [ ] Endpoints should support the parameters stated in API reference (towards OpenAI Compatibility)
- [ ] https://cortex.so/api-reference is updated

## Endpoints
### Chat Completions
- [ ] POST `v1/chat/completions`
- [ ] Cortex supports Function Calling #295

### Engines
- [ ] List engines: GET `/v1/engines`
- [ ] Get engine: GET `/v1/engines/{name}`
- [ ] Install engine: POST `/v1/engines/install/{name}`
- [ ] Get default engine variant/version: GET `v1/engines/{name}/default`
- [ ] Set default engine variant/version: POST `v1/engines/{name}/default`
- [ ] Load engine: POST `v1/engines/{name}/load`
- [ ] Unload engine: DELETE `v1/engines/{name}/load`
- [ ] Update engine: POST `v1/engines/{name}/update`
- [ ] uninstall engine: DELETE `/v1/engines/install/{name}`

### Pulling Models
- [ ] Pull model: POST `/v1/models/pull` starts download (websockets)
- [ ] Pull model: `websockets /events` emitted  
- [ ] Stop model download: DELETE `/v1/models/pull` (websockets)
- [ ] Stop model download: `websockets /events` stopped
- [ ] Import model: POST `v1/models/import`

### Running Models
- [ ] List models: GET `v1/models`
- [ ] Start model: POST `/v1/models/start`
- [ ] Stop model: POST `/v1/models/stop`
- [ ] Get model: GET `/v1/models/{id}`
- [ ] Delete model: DELETE `/v1/models/{id}`
- [ ] Update model: PATCH `/v1/models/{model}` updates model.yaml params

## Server
- [ ] CORs [WIP]
- [ ] health: GET `/healthz`
- [ ] terminate server: DELETE `/processManager/destroy`
--------
Test list for reference:
- #1357 e2e tests for APIs in CI
- #1147, #1225 for starting QA list