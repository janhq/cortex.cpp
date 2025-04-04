// @ts-check
import { defineConfig } from "astro/config";
import starlight from "@astrojs/starlight";
import starlightUtils from "@lorenzo_lewis/starlight-utils";
import starlightVideos from 'starlight-videos'


// https://astro.build/config
export default defineConfig({
  site: 'https://cortex.so',
  integrations: [
    starlight({
      title: "Cortex",
      logo: { src: "./src/assets/logos/cortex.svg" },
      favicon: "./src/assets/logos/cortex.svg",
      social: {
        github: "https://github.com/menloresearch/cortex.cpp",
        discord: "https://discord.gg/FTk2MvZwJH",
        "x.com": "https://x.com/cortex_so",
        youtube: "https://www.youtube.com/@menloresearch",
        linkedin: "https://www.linkedin.com/company/menloresearch/",
      },
      sidebar: [
        {
          label: "Title Bar",
          items: [
            { label: "Docs", link: "overview" },
            { label: "Model Hub", link: "/model-hub" },
          ],
        },
        { label: "Welcome", slug: "overview" },
        {
          label: "How-To",
          items: [
            { label: "Get Started", slug: "how-to/quickstart" },
            { label: "Install Cortex", slug: "how-to/installation" },
            { label: "Use Models", slug: "how-to/using-models" },
            { label: "Troubleshoot", slug: "how-to/troubleshooting" },
            { label: "Configuration", 
              items: [
                { label: "Overview", slug: "how-to/configurations" },
                { label: "CORS", slug: "how-to/configurations/cors" },
                { label: "Proxy", slug: "how-to/configurations/proxy" },
                { label: "Token", slug: "how-to/configurations/token" },
              ],
            },
          ],
        },
        {
          label: "Explanation",
          items: [
            { label: "Architecture", 
              items: [
                // { label: "Overview", slug: "explanation/architecture" },
                { label: "Cortex DB", slug: "explanation/architecture/cortex-db" },
                { label: ".cortexrc", slug: "explanation/architecture/cortexrc" },
                { label: "Data Folder", slug: "explanation/architecture/data-folder" },
                { label: "Updater", slug: "explanation/architecture/updater" },
              ],
            },
            {
              label: "Models",
              items: [
                { label: "Overview", slug: "explanation/models" },
                { label: "Model YAML", slug: "explanation/models/model-yaml" }
                // { label: "Hardware", slug: "capabilities/hardware" },
              ],
            },
            {
              label: "Engines",
              items: [
                { label: "Overview", slug: "explanation/engines" },
                { label: "Model YAML", slug: "explanation/engines/llamacpp" }
                // { label: "Hardware", slug: "capabilities/hardware" },
              ],
            },
          ],
        },
        {
          label: "Tutorials",
          items: [
            { label: "Text Generation", slug: "tutorials/text-generation" },
            { label: "Embeddings", slug: "tutorials/embeddings" },
            { label: "Assistants", slug: "tutorials/assistants" },
            { label: "Function Calling", slug: "tutorials/function-calling" },
            { label: "Structured Outputs", slug: "tutorials/structured-outputs" },
          ],
        },
        {
          label: "Reference",
          items: [
          {
            label: "CLI",
            collapsed: true,
            items: [
              { label: "cortex", slug: "reference/cli" },
              { label: "cortex start", slug: "reference/cli/start" },
              { label: "cortex run", slug: "reference/cli/run" },
              { label: "cortex config", slug: "reference/cli/config" },
              { label: "cortex pull", slug: "reference/cli/pull" },
              { label: "cortex stop", slug: "reference/cli/stop" },
              { label: "cortex ps", slug: "reference/cli/ps" },
              { label: "cortex serve", slug: "reference/cli/serve" },
              { label: "cortex update", slug: "reference/cli/update" },
              { label: "cortex hardware", slug: "reference/cli/hardware" },
            ]
          },
          {
            label: "Models",
            collapsed: true,
            items: [
              { label: "Overview", slug: "reference/cli/models" },
              { label: "cortex models list", slug: "reference/cli/models/list" },
              { label: "cortex models get", slug: "reference/cli/models/get" },
              { label: "cortex models pull", slug: "reference/cli/models/pull" },
              { label: "cortex models start", slug: "reference/cli/models/start" },
              { label: "cortex models stop", slug: "reference/cli/models/stop" },
              { label: "cortex models update", slug: "reference/cli/models/update" },
              { label: "Remove", slug: "reference/cli/models/remove" },
            ],
          },
            {
              label: "Configs",
              collapsed: true,
              items: [
                { label: "Overview", slug: "reference/cli/configs" },
                { label: "List", slug: "reference/cli/configs/list" },
                { label: "Get", slug: "reference/cli/configs/get" },
                { label: "Set", slug: "reference/cli/configs/set" },
              ],
            },
            {
              label: "Engines",
              collapsed: true,
              items: [
                { label: "Overview", slug: "reference/cli/engines" },
                { label: "List", slug: "reference/cli/engines/list" },
                { label: "Get", slug: "reference/cli/engines/get" },
                { label: "Init", slug: "reference/cli/engines/init" },
              ],
            },
          ],
        },
      ],
      plugins: [
        starlightUtils({
          navLinks: {
            leading: { useSidebarLabelled: "Title Bar" },
          },
        }),
        starlightVideos(),
      ],
    }),
  ],
});
