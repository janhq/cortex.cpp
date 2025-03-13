require("dotenv").config();

import { themes as prismThemes } from "prism-react-renderer";
import type { Config } from "@docusaurus/types";
import type * as Preset from "@docusaurus/preset-classic";
import type { ScalarOptions } from "@scalar/docusaurus";
import { downloadFile, listModels, listFiles } from "@huggingface/hub";
import { remarkCodeHike } from "@code-hike/mdx";
import fs from "fs";
import path from "path";
import matter from "gray-matter";

const date = new Date();

const month = ("0" + (date.getMonth() + 1)).slice(-2);
const day = ("0" + date.getDate()).slice(-2);
const year = date.getFullYear();

const formattedDate = `${month}-${day}-${year}`;

async function fetchDataDaily(date: string) {
  const response = await fetch(
    `https://delta.jan.ai/openai-api-collection-test/${date}.json`
  );
  if (!response.ok) {
    return {};
  }
  const data = await response.json();
  return data;
}

function generateDates(startDate: string, numberOfDays: number): string[] {
  const dates: string[] = [];
  const start = new Date(startDate);

  for (let i = 0; i < numberOfDays; i++) {
    const date = new Date(start);
    date.setDate(start.getDate() - i);
    const formattedDate = `${(date.getMonth() + 1)
      .toString()
      .padStart(2, "0")}-${date
      .getDate()
      .toString()
      .padStart(2, "0")}-${date.getFullYear()}`;
    dates.push(formattedDate);
  }

  return dates;
}

const dateArray = generateDates(formattedDate, 30);

const config: Config = {
  title: "Cortex",
  titleDelimiter: "-",
  tagline:
    "Cortex is an Local AI engine for developers to run and customize Local LLMs. It is packaged with a Docker-inspired command-line interface and a Typescript client library. It can be used as a standalone server, or imported as a library. Cortex's roadmap is to eventually support full OpenAI API-equivalence.",
  favicon: "img/favicons/favicon.ico",
  staticDirectories: ["static"],

  plugins: [
    [
      "@docusaurus/plugin-content-docs",
      {
        id: "changelog",
        path: "changelog",
        routeBasePath: "changelog",
      },
    ],
    "docusaurus-plugin-sass",
    async function myPlugin(context, options) {
      return {
        name: "docusaurus-tailwindcss",
        configurePostCss(postcssOptions) {
          // Appends TailwindCSS and AutoPrefixer.
          postcssOptions.plugins.push(require("tailwindcss"));
          postcssOptions.plugins.push(require("autoprefixer"));
          return postcssOptions;
        },
      };
    },

    async function modelsPagesGenPlugin(context, options) {
      return {
        name: "list-models",
        async contentLoaded({ content, actions }) {
          const { setGlobalData } = actions;
          try {
            let fetchedModels = [];
            for await (const model of listModels({
              search: { owner: "cortexso" },
            })) {
              try {
                const files = [];
                let readmeContent = "README.md not available";
                let modelContent = "model.yml not available";
                for await (const fileInfo of listFiles({
                  repo: model.name,
                })) {
                  files.push(fileInfo);
                  if (fileInfo.path === "README.md") {
                    const response = await downloadFile({
                      repo: model.name,
                      path: "README.md",
                    });
                    if (response && response.text) {
                      readmeContent = await response.text();
                    }
                  }
                  if (fileInfo.path === "model.yml") {
                    const response = await downloadFile({
                      repo: model.name,
                      path: "model.yml",
                    });
                    if (response && response.text) {
                      modelContent = await response.text();
                    }
                  }
                }
                try {
                  let refs = {};
                  const response = await fetch(
                    `https://huggingface.co/api/models/${model.name}/refs`
                  );
                  refs = await response.json();
                  fetchedModels.push({
                    ...model,
                    files,
                    readmeContent,
                    modelContent,
                    ...refs,
                  });
                } catch (error) {
                  console.error("Error fetching refs");
                }
              } catch (error) {
                console.error("Error fetching files:", error);
                fetchedModels.push({
                  ...model,
                  files: [],
                  readmeContent: "Error fetching README.md",
                  modelContent: "Error fetching model.yml",
                  error: "Error fetching files",
                });
              }
            }
            setGlobalData(fetchedModels);
            await Promise.all(
              fetchedModels.map(async (page) => {
                return actions.addRoute({
                  // this is the path slug
                  // you can make it dynamic here
                  path: `/models/${page.name.replace("cortexso/", "")}`,
                  // the page component used to render the page
                  component: require.resolve(
                    "./src/components/MyModelPage/index.tsx"
                  ),
                  // will only match for exactly matching paths
                  exact: true,
                  // you can use this to optionally overwrite certain theme components
                  // see here: https://github.com/facebook/docusaurus/blob/main/packages/docusaurus-plugin-content-blog/src/index.ts#L343
                  modules: {},
                  // any extra custom data keys are passed to the page
                  // in this case, we merge the page data together with the loaded content data
                  customData: { ...page },
                });
              })
            );
          } catch (error) {
            console.error("Error fetching models:", error);
          }
        },
      };
    },

    async function getChangelogList(context, options) {
      return {
        name: "changelog-list",
        async contentLoaded({ content, actions }) {
          const { setGlobalData } = actions;

          let changelog = [];

          const changelogDir = path.resolve(__dirname, "changelog");
          const files = fs.readdirSync(changelogDir);

          // Loop through all .mdx files in the changelog directory
          files.forEach(async (file) => {
            if (file.endsWith(".mdx")) {
              const filePath = path.join(changelogDir, file);
              const fileContent = fs.readFileSync(filePath, "utf-8");

              const { data, content } = matter(fileContent);

              changelog.push({
                frontmatter: data, // Frontmatter metadata (e.g., title, date)
                body: content, // The actual MDX content
              });
            }
          });
          changelog.sort(
            (a, b) =>
              new Date(b.frontmatter.date).getTime() -
              new Date(a.frontmatter.date).getTime()
          );
          setGlobalData(changelog);
        },
      };
    },

    async function getRepoInfo(context, options) {
      return {
        name: "repo-info",
        async contentLoaded({ content, actions }) {
          const { setGlobalData } = actions;
          const fetchRepoInfo = await fetch(
            "https://api.github.com/repos/janhq/cortex.cpp"
          );
          const repoInfo = await fetchRepoInfo.json();
          setGlobalData(repoInfo);
        },
      };
    },
    async function getRepoLatestReleaseInfo(context, options) {
      return {
        name: "latest-release",
        async contentLoaded({ content, actions }) {
          const { setGlobalData } = actions;
          const fetchLatestRelease = await fetch(
            "https://api.github.com/repos/janhq/cortex.cpp/releases/latest"
          );
          const latestRelease = await fetchLatestRelease.json();
          setGlobalData(latestRelease);
        },
      };
    },
    async function getDataOAITotalCoverage(context, options) {
      return {
        name: "oai-total-coverage",
        async contentLoaded({ content, actions }) {
          const { setGlobalData } = actions;
          const fetchTotalCoverage = await fetch(
            "https://delta.jan.ai/openai-api-collection-test/total-coverage.json"
          );
          const totalCoverage = await fetchTotalCoverage.json();
          setGlobalData(totalCoverage);
        },
      };
    },
    async function getDataOAIDaily(context, options) {
      return {
        name: "oai-daily-report",
        async contentLoaded({ content, actions }) {
          const { setGlobalData } = actions;

          let results = [];
          for (let date of dateArray) {
            try {
              let data = await fetchDataDaily(date);
              results.push({ date: date, ...data } as never);
            } catch (error) {
              results.push({ date: date } as never);
            }
          }

          setGlobalData(results as []);
        },
      };
    },
    [
      "./src/plugins/scalar/index.ts",
      {
        label: "API Reference",
        showNavLink: false,
        route: "/api-reference",
        configuration: {
          spec: {
            url: "/openapi/cortex.json",
          },
          hideModels: true,
        },
      } as ScalarOptions,
    ],
    "docusaurus-plugin-dotenv",
  ],

  scripts: [
    {
      src: `https://www.googletagmanager.com/gtag/js?id=${process.env.GTM_ID}`,
      async: true,
    },
    {
      src: "/js/gtag.js",
      async: false,
    },
  ],

  // Set the production url of your site here
  url: "https://cortex.so",
  // Set the /<baseUrl>/ pathname under which your site is served
  // For GitHub pages deployment, it is often '/<projectName>/'
  baseUrl: "/",

  themes: ["live-codeblock", "@docusaurus/theme-mermaid"],

  markdown: {
    format: "detect",
    mermaid: true,
  },

  // GitHub pages deployment config.
  // If you aren't using GitHub pages, you don't need these.
  organizationName: "janhq", // Usually your GitHub org/user name.
  projectName: "cortex", // Usually your repo name.

  onBrokenLinks: "throw",
  onBrokenMarkdownLinks: "warn",

  // Even if you don't use internationalization, you can use this field to set
  // useful metadata like html lang. For example, if your site is Chinese, you
  // may want to replace "en" with "zh-Hans".
  i18n: {
    defaultLocale: "en",
    locales: ["en"],
  },

  presets: [
    [
      "classic",
      {
        docs: {
          beforeDefaultRemarkPlugins: [
            [
              remarkCodeHike,
              {
                theme: "dark-plus",
                showCopyButton: true,
                skipLanguages: ["mermaid"],
              },
            ],
          ],
          sidebarPath: "./sidebars.ts",
          // Please change this to your repo.
          // Remove this to remove the "edit this page" links.
          editUrl: "https://github.com/janhq/cortex.cpp/blob/dev/docs/",
        },
        sitemap: {
          changefreq: "daily",
          priority: 1.0,
          ignorePatterns: ["/tags/**"],
          filename: "sitemap.xml",
        },
        theme: {
          customCss: [
            require.resolve("@code-hike/mdx/styles.css"),
            "./src/styles/main.scss",
          ],
        },
      } satisfies Preset.Options,
    ],
  ],

  themeConfig: {
    algolia: {
      appId: process.env.ALGOLIA_APP_ID || "XXX",
      apiKey: process.env.ALGOLIA_API_KEY || "XXX",
      indexName: "cortex",
      contextualSearch: true,
      insights: true,
    },

    metadata: [
      {
        name: "description",
        content:
          "Cortex is an Local AI engine for developers to run and customize Local LLMs. It is packaged with a Docker-inspired command-line interface and a Typescript client library. It can be used as a standalone server, or imported as a library. Cortex's roadmap is to eventually support full OpenAI API-equivalence.",
      },
      {
        name: "og:description",
        content:
          "Cortex is an Local AI engine for developers to run and customize Local LLMs. It is packaged with a Docker-inspired command-line interface and a Typescript client library. It can be used as a standalone server, or imported as a library. Cortex's roadmap is to eventually support full OpenAI API-equivalence.",
      },
    ],

    headTags: [
      // Declare some json-ld structured data
      {
        tagName: "script",
        attributes: {
          type: "application/ld+json",
        },
        innerHTML: JSON.stringify({
          "@context": "https://schema.org/",
          "@type": "Organization",
          name: "Cortex",
          url: "https://cortex.so/",
          logo: "https://cortex.so/img/logos/cortex-logo.svg",
        }),
      },
    ],

    image: "img/social-card.jpg",
    navbar: {
      logo: {
        alt: "Cortex Logo",
        src: "/img/logos/cortex-logo.svg",
        srcDark: "/img/logos/cortex-logo-dark.svg",
        width: 116,
      },
      items: [
        { to: "/models", label: "Models", position: "left" },
        { to: "/changelog", label: "Changelog", position: "left" },
        {
          type: "doc",
          position: "right",
          docId: "overview",
          label: "Docs",
        },
        {
          to: "/api-reference",
          label: "API Reference",
          position: "right",
        },
        {
          type: "search",
          position: "right",
        },
        {
          type: "custom-socialNavbar",
          position: "right",
        },
      ],
    },
    footer: {
      links: [
        {
          title: "Cortex",
          items: [
            {
              label: "Docs",
              to: "/docs",
            },
            { to: "/docs/cli", label: "CLI" },
            { href: "/api-reference", label: "API Reference" },
            { to: "/models", label: "Models" },
            { to: "/changelog", label: "Changelog" },
          ],
        },
        {
          title: "Community",
          items: [
            {
              label: "Github",
              href: "https://github.com/janhq/cortex.cpp",
            },
            {
              label: "Discord",
              href: "https://discord.gg/FTk2MvZwJH",
            },
            {
              label: "Twitter",
              href: "https://x.com/cortex_so",
            },
            {
              label: "Linkedin",
              href: "https://www.linkedin.com/company/homebrewltd/",
            },
          ],
        },
        {
          title: "Company",
          items: [
            {
              label: "About",
              href: "https://jan.ai/about",
            },
            {
              label: "Careers",
              href: "https://menlo.bamboohr.com/careers",
            },
          ],
        },
      ],
      logo: {
        alt: "Cortex Logo",
        src: "/img/logos/cortex-logo-mark.svg",
        srcDark: "/img/logos/cortex-logo-mark.svg",
        width: 34,
      },
      copyright: `©${new Date().getFullYear()} Homebrew Computer Company`,
    },
    prism: {
      theme: prismThemes.github,
      darkTheme: prismThemes.dracula,
    },
  } satisfies Preset.ThemeConfig,
};

export default config;
