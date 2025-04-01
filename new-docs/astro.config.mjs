// @ts-check
import { defineConfig } from "astro/config";
import starlight from "@astrojs/starlight";
import starlightUtils from "@lorenzo_lewis/starlight-utils";

// https://astro.build/config
export default defineConfig({
  integrations: [
    starlight({
      title: "Cortex",
      logo: { src: "./src/assets/logos/cortex.svg" },
      favicon: "./src/assets/logos/cortex.svg",
      social: {
        github: "https://github.com/menloresearch/cortex.cpp",
        discord: "https://github.com/menloresearch/cortex.cpp",
        "x.com": "https://github.com/menloresearch/cortex.cpp",
        youtube: "https://github.com/menloresearch/cortex.cpp",
        linkedin: "https://github.com/menloresearch/cortex.cpp",
      },
      sidebar: [
        {
          label: "Title Bar",
          items: [
            { label: "Docs", link: "overview" },
            { label: "Model Hub", link: "../pages/index" },
          ],
        },
        { label: "Welcome", slug: "overview" },
        {
          label: "Get Started",
          items: [
            { label: "Quickstart", slug: "getting-started/quickstart" },
            {
              label: "Installation",
              items: [
                {
                  label: "Overview",
                  slug: "getting-started/installation/installation",
                },
                {
                  label: "Docker",
                  slug: "getting-started/installation/docker",
                },
                { label: "Linux", slug: "getting-started/installation/linux" },
                { label: "macOS", slug: "getting-started/installation/mac" },
                {
                  label: "Windows",
                  slug: "getting-started/installation/windows",
                },
              ],
            },
            { label: "Requirements", slug: "requirements" },
            { label: "Basic Usage", slug: "basic-usage" },
            { label: "Using Models", slug: "using-models" },
            { label: "Troubleshooting", slug: "troubleshooting" },
          ],
        },
        {
          label: "Features",
          items: [
            { label: "Chat Completions", slug: "chat-completions" },
            {
              label: "Capabilities",
              items: [
                {
                  label: "Text Generation",
                  slug: "capabilities/text-generation",
                },
                { label: "Embeddings", slug: "capabilities/embeddings" },
              ],
            },
            {
              label: "Assistants",
              slug: "assistants",
              items: [{ label: "Tools", slug: "assistants/tools" }],
            },
          ],
        },
        {
          label: "Models",
          items: [
            { label: "Overview", slug: "capabilities/models" },
            { label: "Model YAML", slug: "capabilities/models/model-yaml" },
            {
              label: "Model Sources",
              items: [
                { label: "Overview", slug: "capabilities/models/sources" },
                {
                  label: "Cortex Hub",
                  slug: "capabilities/models/sources/cortex-hub",
                },
                {
                  label: "Hugging Face",
                  slug: "capabilities/models/sources/hugging-face",
                },
                {
                  label: "NVIDIA NGC",
                  slug: "capabilities/models/sources/nvidia-ngc",
                },
              ],
            },
            { label: "Hardware", slug: "capabilities/hardware" },
          ],
        },
        {
          label: "CLI",
          items: [
            { label: "Overview", slug: "cli/cortex" },
            { label: "Run", slug: "cli/run" },
            { label: "Start", slug: "cli/start" },
            { label: "Stop", slug: "cli/stop" },
            { label: "PS", slug: "cli/ps" },
            { label: "Serve", slug: "cli/serve" },
            { label: "Pull", slug: "cli/pull" },
            { label: "Update", slug: "cli/update" },
            { label: "Benchmark", slug: "cli/benchmark" },
            // { label: "Embeddings", slug: "cli/embeddings" },
            { label: "Presets", slug: "cli/presets" },
            { label: "Config", slug: "cli/config" },
            { label: "Telemetry", slug: "cli/telemetry" },
            {
              label: "Models",
              items: [
                { label: "Overview", slug: "cli/models" },
                { label: "List", slug: "cli/models/list" },
                { label: "Get", slug: "cli/models/get" },
                { label: "Download", slug: "cli/models/download" },
                { label: "Start", slug: "cli/models/start" },
                { label: "Stop", slug: "cli/models/stop" },
                { label: "Update", slug: "cli/models/update" },
                { label: "Remove", slug: "cli/models/remove" },
              ],
            },
            {
              label: "Configs",
              items: [
                { label: "Overview", slug: "cli/configs" },
                { label: "List", slug: "cli/configs/list" },
                { label: "Get", slug: "cli/configs/get" },
                { label: "Set", slug: "cli/configs/set" },
              ],
            },
            {
              label: "Engines",
              items: [
                { label: "Overview", slug: "cli/engines" },
                { label: "List", slug: "cli/engines/list" },
                { label: "Get", slug: "cli/engines/get" },
                { label: "Init", slug: "cli/engines/init" },
              ],
            },
            {
              label: "Hardware",
              slug: "cli/hardware",
            },
          ],
        },
        {
          label: "Configurations",
          items: [
            { label: "Overview", slug: "configurations" },
            { label: "CORS", slug: "configurations/cors" },
            { label: "Proxy", slug: "configurations/proxy" },
            { label: "Token", slug: "configurations/token" },
          ],
        },
        {
          label: "Engines",
          items: [
            { label: "Overview", slug: "engines" },
            { label: "LlamaCpp", slug: "engines/llamacpp" },
          ],
        },
        {
          label: "Guides",
          items: [
            { label: "Function Calling", slug: "guides/function-calling" },
            { label: "Structured Outputs", slug: "guides/structured-outputs" },
          ],
        },
        {
          label: "Architecture",
          items: [
            { label: "Overview", slug: "architecture" },
            { label: "Cortex DB", slug: "architecture/cortex-db" },
            { label: "Cortex RC", slug: "architecture/cortexrc" },
            { label: "Data Folder", slug: "architecture/data-folder" },
            { label: "Updater", slug: "architecture/updater" },
            { label: "Benchmarking", slug: "benchmarking-architecture" },
            { label: "Telemetry", slug: "telemetry-architecture" },
          ],
        },
        {
          label: "Cortex Platform",
          items: [
            { label: "About", slug: "cortex-platform/about" },
            { label: "Benchmarking", slug: "cortex-platform/benchmarking" },
          ],
        },
      ],
      plugins: [
        starlightUtils({
          navLinks: {
            leading: { useSidebarLabelled: "Title Bar" },
          },
        }),
      ],
    }),
  ],
});
