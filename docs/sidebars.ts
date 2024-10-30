import type { SidebarsConfig } from "@docusaurus/plugin-content-docs";

/**
 * Creating a sidebar enables you to:
 - create an ordered group of docs
 - render a sidebar for each doc of that group
 - provide next/previous navigation

 The sidebars can be generated from the filesystem, or explicitly defined here.

 Create as many sidebars as you want.
 */
const sidebars: SidebarsConfig = {
  // By default, Docusaurus generates a sidebar from the docs folder structure
  sidebar: [
    // {
    //   type: "html",
    //   value:
    //     '<div class="mt-4"><a class="menu__link" href="/docs/"><svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" class="lucide mr-2 lucide-brain-circuit w-7 h-7 p-1 border rounded border-gray-200 dark:border-gray-700"><path d="M12 5a3 3 0 1 0-5.997.125 4 4 0 0 0-2.526 5.77 4 4 0 0 0 .556 6.588A4 4 0 1 0 12 18Z"></path><path d="M9 13a4.5 4.5 0 0 0 3-4"></path><path d="M6.003 5.125A3 3 0 0 0 6.401 6.5"></path><path d="M3.477 10.896a4 4 0 0 1 .585-.396"></path><path d="M6 18a4 4 0 0 1-1.967-.516"></path><path d="M12 13h4"></path><path d="M12 18h6a2 2 0 0 1 2 2v1"></path><path d="M12 8h8"></path><path d="M16 8V5a2 2 0 0 1 2-2"></path><circle cx="16" cy="13" r=".5"></circle><circle cx="18" cy="3" r=".5"></circle><circle cx="20" cy="21" r=".5"></circle><circle cx="20" cy="8" r=".5"></circle></svg>Cortex</a></div>',
    // },
    // {
    //   type: "html",
    //   value:
    //     '<div><a class="menu__link" href="/docs/cortex-platform/"><svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" class="lucide mr-2 lucide-blocks w-7 h-7 p-1 border rounded border-gray-200 dark:border-gray-700"><rect width="7" height="7" x="14" y="3" rx="1"></rect><path d="M10 21V8a1 1 0 0 0-1-1H4a1 1 0 0 0-1 1v12a1 1 0 0 0 1 1h12a1 1 0 0 0 1-1v-5a1 1 0 0 0-1-1H3"></path></svg>Platform<span class="bg-black dark:bg-white dark:text-black text-white rounded-full px-2 py-1 ml-2 text-xs">Coming Soon</span></a></div>',
    // },
    {
      type: "html",
      value: "GET STARTED",

      className: "sidebar-divider",
    },
    "overview",
    "quickstart",
    {
      type: "category",
      label: "Installation",
      link: {
        type: "generated-index",
      },
      collapsed: true,
      items: [
        { type: "doc", id: "installation/windows", label: "Windows" },
        { type: "doc", id: "installation/mac", label: "Mac" },
        { type: "doc", id: "installation/linux", label: "Linux" },
        { type: "doc", id: "installation/docker", label: "Docker" },
        {
          type: "doc",
          id: "installation/gpu-acceleration",
          label: "GPU Acceleration",
        },
      ],
    },
    {
      type: "html",
      value: "BASIC USAGE",

      className: "sidebar-divider",
    },
    { type: "doc", id: "basic-usage/overview", label: "Overview" },
    { type: "doc", id: "basic-usage/cortexrc", label: ".cortexrc" },
    { type: "doc", id: "model-yaml", label: "model.yaml" },
    { type: "doc", id: "data-folder", label: "Data Folder" },
    {
      type: "category",
      label: "Libraries",
      link: {
        type: "generated-index",
      },
      collapsed: true,
      items: [
        {
          type: "doc",
          id: "basic-usage/integration/js-library",
          label: "cortex.js",
        },
        {
          type: "doc",
          id: "basic-usage/integration/py-library",
          label: "cortex.py",
        },
      ],
    },
    {
      type: "category",
      label: "Model Sources",
      link: { type: "doc", id: "hub/index" },
      collapsed: true,
      items: [
        { type: "doc", id: "hub/cortex-hub", label: "Cortex Model Repos" },
        { type: "doc", id: "hub/hugging-face", label: "HuggingFace Repos" },
        {
          type: "doc",
          id: "hub/nvidia-ngc",
          label: "Nvidia Catalog (Coming Soon)",
        },
      ],
    },
    {
      type: "category",
      label: "Engines",
      link: { type: "doc", id: "engines/index" },
      collapsed: true,
      items: [
        { type: "doc", id: "engines/llamacpp", label: "Llama.cpp" },
        // { type: "doc", id: "engines/tensorrt-llm", label: "TensorRT-LLM" },
        // { type: "doc", id: "engines/onnx", label: "ONNX" },
      ],
    },
    // {
    //   type: "category",
    //   label: "Basic Usage",
    //   link: {
    //     type: "generated-index",
    //   },
    //   collapsed: true,
    //   items: [
    //     { type: "doc", id: "basic-usage/command-line", label: "CLI" },
    //     { type: "doc", id: "basic-usage/server", label: "API" },
    // {
    //   type: "category",
    //   label: "Integration",
    //   link: {
    //     type: "generated-index",
    //   },
    //   collapsed: true,
    //   items: [
    //     {
    //       type: "doc",
    //       id: "basic-usage/integration/js-library",
    //       label: "cortex.js",
    //     },
    //     {
    //       type: "doc",
    //       id: "basic-usage/integration/py-library",
    //       label: "cortex.py",
    //     },
    //   ],
    // },
    //   ],
    // },
    // { type: "doc", id: "telemetry", label: "Telemetry" },
    // MODELs
    // {
    //   type: "html",
    //   value: "MODELS",
    //   className: "sidebar-divider",
    // },
    // { type: "doc", id: "model-overview", label: "Overview" },
    // { type: "doc", id: "model-yaml", label: "model.yaml" },
    // { type: "doc", id: "built-in-models", label: "Built-in Models" },
    // {
    //   type: "category",
    //   label: "Using Models",
    //   link: { type: "doc", id: "using-models" },
    //   collapsed: true,
    //   items: [
    //     { type: "doc", id: "model-yaml", label: "model.yaml" },
    //     // { type: "doc", id: "model-presets", label: "Model Presets" },
    //     { type: "doc", id: "built-in-models", label: "Built-in Models" },
    //   ],
    // },
    // BASIC USAGE
    // {
    //   type: "html",
    //   value: "BASIC USAGE",
    //   className: "sidebar-divider",
    // },
    // { type: "doc", id: "command-line", label: "CLI" },
    // { type: "doc", id: "ts-library", label: "Typescript Library" },
    // { type: "doc", id: "py-library", label: "Python Library" },
    // { type: "doc", id: "server", label: "Server Endpoint" },
    // CAPABILITIES
    // {
    //   type: "html",
    //   value: "ENDPOINTS",
    //   className: "sidebar-divider",
    // },
    // { type: "doc", id: "chat-completions", label: "Chat Completions" },
    // { type: "doc", id: "embeddings", label: "Embeddings" },
    // CLI
    {
      type: "html",
      value: "CLI",
      className: "sidebar-divider",
    },
    { type: "doc", id: "cli/cortex", label: "cortex" },
    { type: "doc", id: "cli/start", label: "cortex start" },
    { type: "doc", id: "cli/chat", label: "cortex chat" },
    // { type: "doc", id: "cli/embeddings", label: "cortex embeddings" },
    // { type: "doc", id: "cli/presets", label: "cortex presets" },
    { type: "doc", id: "cli/pull", label: "cortex pull" },
    { type: "doc", id: "cli/run", label: "cortex run" },
    { type: "doc", id: "cli/models/index", label: "cortex models" },
    { type: "doc", id: "cli/engines/index", label: "cortex engines" },
    { type: "doc", id: "cli/stop", label: "cortex stop" },
    { type: "doc", id: "cli/ps", label: "cortex ps" },
    { type: "doc", id: "cli/update", label: "cortex update" },
    // { type: "doc", id: "cli/telemetry", label: "cortex telemetry" },
    // { type: "doc", id: "cli/benchmark", label: "cortex benchmark" },
    // ARCHITECTURE
    // {
    //   type: "html",
    //   value: "ARCHITECTURE",
    //   className: "sidebar-divider",
    // },
    // { type: "doc", id: "architecture", label: "Cortex" },
    // {
    //   type: "category",
    //   label: "Engines",
    //   link: {
    //     type: "generated-index",
    //   },
    //   collapsed: true,
    //   items: [
    //     { type: "doc", id: "cortex-llamacpp", label: "llama.cpp" },
    //     { type: "doc", id: "cortex-tensorrt-llm", label: "TensorRT-LLM" },
    //     { type: "doc", id: "cortex-onnx", label: "ONNX" },
    //     {
    //       type: "doc",
    //       id: "integrate-remote-engine",
    //       label: "Integrate Remote Engine",
    //     },
    //   ],
    // },
    // {
    //   type: "category",
    //   label: "Infrastructure",
    //   link: {
    //     type: "generated-index",
    //   },
    //   collapsed: true,
    //   items: [
    //     { type: "doc", id: "telemetry-architecture", label: "Telemetry Infra" },
    //     {
    //       type: "doc",
    //       id: "benchmarking-architecture",
    //       label: "Benchmarking Infra",
    //     },
    //   ],
    // },
    // {
    //   type: "html",
    //   value: "TROUBLESHOOTING",
    //   className: "sidebar-divider",
    // },
    // { type: "doc", id: "troubleshooting", label: "Troubleshooting" },
  ],
  platform: [
    {
      type: "html",
      value:
        '<div class="mt-4"><a class="menu__link" href="/docs/"><svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" class="lucide mr-2 lucide-brain-circuit w-7 h-7 p-1 border rounded border-gray-200 dark:border-gray-700"><path d="M12 5a3 3 0 1 0-5.997.125 4 4 0 0 0-2.526 5.77 4 4 0 0 0 .556 6.588A4 4 0 1 0 12 18Z"></path><path d="M9 13a4.5 4.5 0 0 0 3-4"></path><path d="M6.003 5.125A3 3 0 0 0 6.401 6.5"></path><path d="M3.477 10.896a4 4 0 0 1 .585-.396"></path><path d="M6 18a4 4 0 0 1-1.967-.516"></path><path d="M12 13h4"></path><path d="M12 18h6a2 2 0 0 1 2 2v1"></path><path d="M12 8h8"></path><path d="M16 8V5a2 2 0 0 1 2-2"></path><circle cx="16" cy="13" r=".5"></circle><circle cx="18" cy="3" r=".5"></circle><circle cx="20" cy="21" r=".5"></circle><circle cx="20" cy="8" r=".5"></circle></svg>Cortex</a></div>',
    },
    {
      type: "html",
      value:
        '<div><a class="menu__link" href="/docs/cortex-platform/"><svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" class="lucide mr-2 lucide-blocks w-7 h-7 p-1 border rounded border-gray-200 dark:border-gray-700"><rect width="7" height="7" x="14" y="3" rx="1"></rect><path d="M10 21V8a1 1 0 0 0-1-1H4a1 1 0 0 0-1 1v12a1 1 0 0 0 1 1h12a1 1 0 0 0 1-1v-5a1 1 0 0 0-1-1H3"></path></svg>Platform<span class="bg-black dark:bg-white dark:text-black text-white rounded-full px-2 py-1 ml-2 text-xs">Coming Soon</span></a></div>',
    },
    {
      type: "html",
      value: "GET STARTED",
      className: "sidebar-divider",
    },
    "cortex-platform/about",
    {
      type: "html",
      value: "ENDPOINTS",
      className: "sidebar-divider",
    },
    { type: "doc", id: "cortex-platform/benchmarking", label: "Benchmarking" },
    {
        type: "html",
        value: "ARCHITECTURE",
        className: "sidebar-divider",
      },
      { type: "doc", id: "architecture", label: "Cortex" },
      {
        type: "category",
        label: "Engines",
        link: {
          type: "generated-index",
        },
        collapsed: true,
        items: [
          { type: "doc", id: "cortex-llamacpp", label: "llama.cpp" },
          { type: "doc", id: "cortex-tensorrt-llm", label: "TensorRT-LLM" },
          { type: "doc", id: "cortex-onnx", label: "ONNX" },
          {
            type: "doc",
            id: "integrate-remote-engine",
            label: "Integrate Remote Engine",
          },
        ],
      },
    {
      type: "category",
      label: "Infrastructure",
      link: {
        type: "generated-index",
      },
      collapsed: true,
      items: [
        { type: "doc", id: "telemetry-architecture", label: "Telemetry Infra" },
        {
          type: "doc",
          id: "benchmarking-architecture",
          label: "Benchmarking Infra",
        },
      ],
    },
    {
      type: "html",
      value: "CLI",
      className: "sidebar-divider",
    },
    // { type: "doc", id: "cli/cortex", label: "cortex" },
    // { type: "doc", id: "cli/chat", label: "cortex chat" },
    // { type: "doc", id: "cli/embeddings", label: "cortex embeddings" },
    { type: "doc", id: "cli/presets", label: "cortex presets" },
    // { type: "doc", id: "cli/pull", label: "cortex pull" },
    // { type: "doc", id: "cli/run", label: "cortex run" },
    // { type: "doc", id: "cli/models/index", label: "cortex models" },
    // { type: "doc", id: "cli/engines/index", label: "cortex engines" },
    // { type: "doc", id: "cli/stop", label: "cortex stop" },
    // { type: "doc", id: "cli/ps", label: "cortex ps" },
    // { type: "doc", id: "cli/telemetry", label: "cortex telemetry" },
    { type: "doc", id: "cli/benchmark", label: "cortex benchmark" },
  ],
};

export default sidebars;
