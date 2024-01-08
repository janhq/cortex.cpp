// @ts-check
// Note: type annotations allow type checking and IDEs autocompletion

require("dotenv").config();

const codeTheme = require("prism-react-renderer/themes/dracula");

/** @type {import('@docusaurus/types').Config} */
const config = {
  title: "Nitro",
  tagline: "Fast inference engine",
  favicon: "img/favicon.ico",

  // Set the production url of your site here
  url: "https://nitro.jan.ai",
  // Set the /<baseUrl>/ pathname under which your site is served
  // For GitHub pages deployment, it is often '/<projectName>/'
  baseUrl: "/",

  // GitHub pages deployment config.
  // If you aren't using GitHub pages, you don't need these.
  organizationName: "janhq", // Usually your GitHub org/user name.
  projectName: "nitro", // Usually your repo name.

  onBrokenLinks: "warn",
  onBrokenMarkdownLinks: "warn",

  // Even if you don't use internalization, you can use this field to set useful
  // metadata like html lang. For example, if your site is Chinese, you may want
  // to replace "en" with "zh-Hans".
  i18n: {
    defaultLocale: "en",
    locales: ["en"],
  },

  markdown: {
    mermaid: true,
  },
  // Plugins we added
  plugins: [
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
    [
      "posthog-docusaurus",
      {
        apiKey: process.env.POSTHOG_PROJECT_API_KEY || "XXX",
        appUrl: process.env.POSTHOG_APP_URL || "XXX", // optional
        enableInDevelopment: false, // optional
      },
    ],
  ],

  // The classic preset will relay each option entry to the respective sub plugin/theme.
  presets: [
    [
      "classic",
      /** @type {import('@docusaurus/preset-classic').Options} */
      ({
        // Will be passed to @docusaurus/plugin-content-docs (false to disable)
        docs: {
          routeBasePath: "/",
          sidebarPath: "./sidebars.js",
          editUrl: "https://github.com/janhq/nitro/tree/main/docs",
          showLastUpdateAuthor: true,
          showLastUpdateTime: true,
        },
        // Will be passed to @docusaurus/plugin-content-sitemap (false to disable)
        sitemap: {
          changefreq: "daily",
          priority: 1.0,
          ignorePatterns: ["/tags/**"],
          filename: "sitemap.xml",
        },
        // Will be passed to @docusaurus/plugin-content-blog (false to disable)
        blog: {
          blogSidebarTitle: "All Posts",
          blogSidebarCount: "ALL",
        },
        // Will be passed to @docusaurus/theme-classic.
        theme: {
          customCss: "./src/styles/main.scss",
        },
        googleTagManager: {
          containerId: process.env.GTM_ID || "XXX",
        },
        // Will be passed to @docusaurus/plugin-content-pages (false to disable)
        // pages: {},
      }),
    ],
    // Redoc preset
    [
      "redocusaurus",
      {
        specs: [
          {
            spec: "openapi/NitroAPI.yaml", // can be local file, url, or parsed json object
            // spec: "openapi/OpenAIAPI.yaml",
            route: "/api-reference/",
          },
        ],
        theme: {
          primaryColor: "#1a73e8",
          primaryColorDark: "#1a73e8",
          // redocOptions: { hideDownloadButton: false },
        },
      },
    ],
  ],

  // Docs: https://docusaurus.io/docs/api/themes/configuration
  themeConfig:
    /** @type {import('@docusaurus/preset-classic').ThemeConfig} */
    ({
      image: "img/nitro-social-card.png",
      // Only for react live
      liveCodeBlock: {
        playgroundPosition: "bottom",
      },
      metadata: [
        {
          name: "description",
          content:
            "Nitro is a high-efficiency Large Language Model inference engine for edge computing.",
        },
        {
          name: "keywords",
          content:
            "Nitro, Jan, fast inference, inference server, local AI, large language model, OpenAI compatible, open source, llama",
        },

        // Canonical URL
        { name: "canonical", content: "https://nitro.jan.ai/" },

        // Robots tags
        { name: "robots", content: "index, follow" },

        // Open Graph tags
        { property: "og:title", content: "Fast inference engine | Nitro" },
        {
          property: "og:description",
          content:
            "Nitro is a high-efficiency Large Language Model inference engine for edge computing.",
        },
        { property: "og:type", content: "website" },

        // Twitter card tags
        { property: "twitter:card", content: "summary_large_image" },
        { property: "twitter:site", content: "@janhq_" },
        { property: "twitter:title", content: "Fast inference engine | Nitro" },
        {
          property: "twitter:description",
          content:
            "Nitro is a high-efficiency Large Language Model inference engine for edge computing.",
        },
      ],
      headTags: [
        // Declare a <link> preconnect tag
        {
          tagName: "link",
          attributes: {
            rel: "preconnect",
            href: "https://nitro.jan.ai/",
          },
        },
        // Declare some json-ld structured data
        {
          tagName: "script",
          attributes: {
            type: "application/ld+json",
          },
          innerHTML: JSON.stringify({
            "@context": "https://schema.org/",
            "@type": "LLMInference",
            name: "Nitro",
            description:
              "Nitro is a high-efficiency Large Language Model inference engine for edge computing.",
            keywords:
              "Nitro, OpenAI compatible, fast inference, local AI, llm, small AI, free, open source, production ready",
            applicationCategory: "BusinessApplication",
            operatingSystem: "Multiple",
            url: "https://nitro.jan.ai/",
          }),
        },
      ],
      navbar: {
        title: "Nitro",
        logo: {
          alt: "Nitro Logo",
          src: "img/logos/nitro.svg",
        },
        items: [
          // Navbar left
          {
            type: "docSidebar",
            sidebarId: "docsSidebar",
            position: "left",
            label: "Documentation",
          },
          {
            type: "docSidebar",
            sidebarId: "apiSidebar",
            position: "left",
            label: "API Reference",
          },
          // {
          //   type: "docSidebar",
          //   sidebarId: "communitySidebar",
          //   position: "left",
          //   label: "Community",
          // },
          // Navbar right
          // {
          //   type: "docSidebar",
          //   sidebarId: "blogSidebar",
          //   position: "right",
          //   label: "Blog",
          // },
        ],
      },
      prism: {
        theme: codeTheme,
        darkTheme: codeTheme,
        additionalLanguages: ["python", "powershell", "bash"],
      },
      colorMode: {
        defaultMode: "dark",
        disableSwitch: false,
        respectPrefersColorScheme: false,
      },
    }),
  // Only for react live
  themes: ["@docusaurus/theme-live-codeblock", "@docusaurus/theme-mermaid"],
};

module.exports = config;
