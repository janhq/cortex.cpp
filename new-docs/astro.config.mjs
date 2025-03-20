// @ts-check
import { defineConfig } from "astro/config";
import starlight from "@astrojs/starlight";

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
        { label: "Welcome", slug: "overview" },
        {
          label: "Get Started",
          items: [
            // Each item here is one entry in the navigation menu.

            { label: "Quickstart", slug: "getting-started/quickstart" },
            {
              label: "Installation",
              items: [
                {
                  label: "Install",
                  slug: "getting-started/installation/installation",
                },
                {
                  label: "Docker",
                  slug: "getting-started/installation/docker",
                },
              ],
            },
            // { label: "Docker", slug: "getting-started/installation/docker" },
            // { label: "Overview", slug: "overview" },
            // { label: "Overview", slug: "overview" },
            // { label: "Overview", slug: "overview" },
            // { label: "Overview", slug: "overview" },
          ],
        },
        {
          label: "Guides",
          items: [
            // Each item here is one entry in the navigation menu.
            { label: "Example Guide", slug: "guides/example" },
          ],
        },
        {
          label: "Reference",
          autogenerate: { directory: "reference" },
        },
      ],
    }),
  ],
});
