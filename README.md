# Cortex Monorepo

This monorepo contains two projects: CortexJS and CortexCPP.

## CortexJS: Stateful Business Backend

* All of the stateful endpoints:
	+ /threads
	+ /messages
	+ /models
	+ /runs
	+ /vector_store
	+ /settings
	+ /?auth
	+ …
* Database & Filesystem
* API Gateway
* Authentication & Authorization
* Observability

## CortexCPP: Stateless Embedding Backend

* All of the high performance, stateless endpoints:
	+ /chat/completion
	+ /audio
	+ /fine_tuning
	+ /embeddings
	+ /load_model
	+ /unload_model
* Kernel - Hardware Recognition

## Project Structure

```
.
├── cortex-js/
│   ├── package.json
│   ├── README.md
│   ├── Dockerfile
│   ├── docker-compose.yml
│   ├── src/
│   │   ├── controllers/
│   │   ├── modules/
│   │   ├── services/
│   │   └── ...
│   └── ...
├── cortex-cpp/
│   ├── app/
│   │   ├── controllers/
│   │   ├── models/
│   │   ├── services/
│   │   ├── ?engines/
│   │   │   ├── llama.cpp
│   │   │   ├── tensorrt-llm
│   │   │   └── ...
│   │   └── ...
│   ├── CMakeLists.txt
│   ├── config.json
│   ├── Dockerfile
│   ├── docker-compose.yml
│   ├── README.md
│   └── ...
├── scripts/
│   └── ...
├── README.md
├── package.json
├── Dockerfile
├── docker-compose.yml
└── docs/
    └── ...
```

## Installation

### NPM Install

* Pre-install script:
```bash
npm pre-install script; platform specific (MacOS / Windows / Linux)
```
* Tag based:
```json
npm install @janhq/cortex
npm install @janhq/cortex#cuda
npm install @janhq/cortex#cuda-avx512
npm install @janhq/cortex#cuda-avx
```

### CLI Install Script

```bash
cortex init (AVX2 + Cuda)

Enable GPU Acceleration?
1. Nvidia (default) - detected
2. AMD
3. Mac Metal

Enter your choice: 

CPU Instructions
1. AVX2 (default) - Recommend based on what the user has
2. AVX  (old CPU)
3. AVX512

Enter your choice: 

Downloading cortex-cuda-avx.so........................25%

Cortex is ready!

It seems like you have installed models from other applications. Do you want to import them?
1. Import from /Users/HOME/jan/models
2. Import from /Users/HOME/lmstudio/models
3. Import everything

Importing from /Users/HOME/jan/models..................17%
```

## Backend (jan app)

```json
POST /settings
{
    "gpu_enabled": true,
    "gpu_family": "Nvidia",
    "cpu_instructions": "AVX2" 
}
```

## Client Library Configuration

TBD