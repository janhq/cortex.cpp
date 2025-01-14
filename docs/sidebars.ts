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
        type: "doc",
        id: "installation",
      },
      collapsed: true,
      items: [
        { type: "doc", id: "installation/windows", label: "Windows" },
        { type: "doc", id: "installation/mac", label: "Mac" },
        { type: "doc", id: "installation/linux", label: "Linux" },
        { type: "doc", id: "installation/docker", label: "Docker" },
      ],
    },
    {
      type: "category",
      label: "Basic Usage",
      link: { type: "doc", id: "basic-usage/index" },
      collapsed: true,
      items: [
        {
          type: "doc",
          id: "basic-usage/cortex-js",
          label: "cortex.js",
        },
        {
          type: "doc",
          id: "basic-usage/cortex-py",
          label: "cortex.py",
        },
      ],
    },
    {
      type: "category",
      label: "Architecture",
      link: {
        type: "generated-index",
        // type: "doc",
        // id: "architecture" // is outdated
      },
      collapsed: true,
      items: [
        {
          type: "doc",
          id: "architecture/data-folder",
          label: "Cortex Data Folder",
        },
        { type: "doc", id: "architecture/cortex-db", label: "cortex.db" },
        { type: "doc", id: "architecture/cortexrc", label: ".cortexrc" },
        { type: "doc", id: "architecture/updater", label: "Updater" },
      ],
    },
    {
      type: "category",
      label: "Configurations",
      link: { type: "doc", id: "configurations/index" },
      collapsed: true,
      items: [
        {
          type: "doc",
          id: "configurations/cors",
          label: "CORS",
        },
        {
          type: "doc",
          id: "configurations/proxy",
          label: "Proxy",
        },
        {
          type: "doc",
          id: "configurations/token",
          label: "Token",
        },
      ],
    },
    {
      type: "html",
      value: "CAPABILITIES",
      className: "sidebar-divider",
    },
    {
      type: "category",
      label: "Pulling Models",
      link: { type: "doc", id: "capabilities/models/sources/index" },
      collapsed: true,
      items: [
        // { type: "doc", id: "capabilities/models/sources/hugging-face", label: "Hugging Face" },
        // { type: "doc", id: "capabilities/models/sources/cortex-hub", label: "Cortex Model Repos" },
        // { type: "doc", id: "capabilities/models/sources/nvidia-ngc", label: "Nvidia Catalog (Coming Soon)"},
      ],
    },
    {
      type: "category",
      label: "Running Models",
      link: { type: "doc", id: "capabilities/models/index" },
      collapsed: true,
      items: [
        {
          type: "doc",
          id: "capabilities/models/model-yaml",
          label: "model.yaml",
        },
        {
          type: "doc",
          id: "capabilities/models/presets",
          label: "Model Presets",
        },
      ],
    },
    {
      type: "category",
      label: "Engine Management",
      link: { type: "doc", id: "engines/index" },
      collapsed: true,
      items: [
        { type: "doc", id: "engines/llamacpp", label: "llama.cpp" },
        { type: "doc", id: "engines/python-engine", label: "python engine" },
        // { type: "doc", id: "engines/tensorrt-llm", label: "TensorRT-LLM" },
        // { type: "doc", id: "engines/onnx", label: "ONNX" },
        {
          type: "doc",
          id: "engines/engine-extension",
          label: "Building Engine Extensions",
        },
      ],
    },
    {
      type: "category",
      label: "Hardware Awareness",
      link: { type: "doc", id: "capabilities/hardware/index" },
      collapsed: true,
      items: [],
    },
    {
      type: "doc",
      id: "capabilities/text-generation",
      label: "Text Generation",
    },
    // { type: "doc", id: "capabilities/image-generation", label: "Image Generation" },
    // { type: "doc", id: "capabilities/vision", label: "Vision" },
    // { type: "doc", id: "capabilities/audio-generation", label: "Audio Generation" },
    // { type: "doc", id: "capabilities/text-to-speech", label: "Text to Speech" },
    // { type: "doc", id: "capabilities/speech-to-text", label: "Speech to text" },
    { type: "doc", id: "capabilities/embeddings", label: "Embeddings" },
    // { type: "doc", id: "capabilities/moderation", label: "Moderation" },
    // { type: "doc", id: "capabilities/reasoning", label: "Reasoning" },
    {
      type: "html",
      value: "GUIDES",
      className: "sidebar-divider",
    },
    { type: "doc", id: "guides/function-calling", label: "Function Calling" },
    {
      type: "doc",
      id: "guides/structured-outputs",
      label: "Structured Outputs",
    },
    {
      type: "html",
      value: "ASSISTANTS",
      className: "sidebar-divider",
    },
    { type: "doc", id: "assistants/index", label: "Assistants" },
    {
      type: "category",
      label: "Tools",
      link: { type: "doc", id: "assistants/tools/index" },
      collapsed: true,
      items: [
        // { type: "doc", id: "assistants/tools/file-search", label: "File Search" },
      ],
    },
    {
      type: "html",
      value: "CLI",
      className: "sidebar-divider",
    },
    { type: "doc", id: "cli/cortex", label: "cortex" },
    { type: "doc", id: "cli/start", label: "cortex start" },
    { type: "doc", id: "cli/run", label: "cortex run" },
    { type: "doc", id: "cli/config", label: "cortex config" },
    // { type: "doc", id: "cli/embeddings", label: "cortex embeddings" },
    // { type: "doc", id: "cli/presets", label: "cortex presets" },
    { type: "doc", id: "cli/pull", label: "cortex pull" },
    { type: "doc", id: "cli/models/index", label: "cortex models" },
    { type: "doc", id: "cli/engines/index", label: "cortex engines" },
    { type: "doc", id: "cli/ps", label: "cortex ps" },
    { type: "doc", id: "cli/update", label: "cortex update" },
    { type: "doc", id: "cli/stop", label: "cortex stop" },
  ],
};

export default sidebars;
