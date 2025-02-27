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
    { type: "html", value: "GET STARTED", className: "sidebar-divider" },
    "overview",
    "quickstart",
    {
      type: "category",
      label: "Installation",
      link: { type: "doc", id: "installation" },
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
        { type: "doc", id: "basic-usage/cortex-js", label: "cortex.js" },
        { type: "doc", id: "basic-usage/cortex-py", label: "cortex.py" },
      ],
    },
    {
      type: "category",
      label: "Architecture",
      link: { type: "generated-index" },
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
        { type: "doc", id: "configurations/cors", label: "CORS" },
        { type: "doc", id: "configurations/proxy", label: "Proxy" },
        { type: "doc", id: "configurations/token", label: "Token" },
      ],
    },
    { type: "html", value: "CAPABILITIES", className: "sidebar-divider" },
    {
      type: "category",
      label: "Pulling Models",
      link: { type: "doc", id: "capabilities/models/sources/index" },
      collapsed: true,
      items: [],
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
      type: "category",
      label: "Deployment",
      link: { type: "doc", id: "guides/deployment/index" },
      collapsed: true,
      items: [
        { type: "doc", id: "guides/deployment/flyio", label: "Fly.io" },
        { type: "doc", id: "guides/deployment/modal", label: "Modal Labs" },
        {
          type: "doc",
          id: "guides/deployment/raspberrypi",
          label: "Raspberry Pi",
        },
      ],
    },
    { type: "html", value: "ASSISTANTS", className: "sidebar-divider" },
    { type: "doc", id: "assistants/index", label: "Assistants" },
    {
      type: "category",
      label: "Tools",
      link: { type: "doc", id: "assistants/tools/index" },
      collapsed: true,
      items: [],
    },
    { type: "html", value: "CLI", className: "sidebar-divider" },
    { type: "doc", id: "cli/cortex", label: "cortex" },
    { type: "doc", id: "cli/start", label: "cortex start" },
    { type: "doc", id: "cli/run", label: "cortex run" },
    { type: "doc", id: "cli/config", label: "cortex config" },
    { type: "doc", id: "cli/pull", label: "cortex pull" },
    { type: "doc", id: "cli/models/index", label: "cortex models" },
    { type: "doc", id: "cli/engines/index", label: "cortex engines" },
    { type: "doc", id: "cli/ps", label: "cortex ps" },
    { type: "doc", id: "cli/update", label: "cortex update" },
    { type: "doc", id: "cli/stop", label: "cortex stop" },
  ],
};

export default sidebars;
