/**
 * Creating a sidebar enables you to:
 - create an ordered group of docs
 - render a sidebar for each doc of that group
 - provide next/previous navigation

 The sidebars are explicitly defined here.

 Create as many sidebars as you want.
 */

// @ts-check

/** @type {import('@docusaurus/plugin-content-docs').SidebarsConfig} */
const sidebars = {
  // blogSidebar: [
  //   "guides/overview"
  // ],

  docsSidebar: [
    {
      type: "category",
      label: "Introduction",
      collapsible: false,
      collapsed: false,
      items: [
        { type: "doc", id: "new/about", label: "About Nitro" },
        { type: "doc", id: "new/quickstart", label: "Quickstart" },
        { type: "doc", id: "new/install", label: "Installation" },
        "new/build-source",
      ],
    },
    {
      type: "category",
      label: "Features",
      collapsible: false,
      collapsed: false,
      link: { type: "doc", id: "features/feat" },
      items: [
        "features/chat",
        "features/embed",
      ],
    },
    {
      type: "category",
      label: "Advanced Features",
      link: { type: "doc", id: "features/feat" },
      items: [
        "features/multi-thread",
        "features/cont-batch",
        "features/load-unload",
        "features/warmup",
        "features/prompt",
      ],
    },
    {
      type: "category",
      label: "Integrations",
      collapsible: false,
      collapsed: false,
      items: [
        "examples/jan",
        // "examples/chatbox",
        "examples/palchat",
        "examples/openai-node",
        "examples/openai-python",
        "examples/colab",
        "examples/chatboxgpt"
      ],
    },
    // {
    //   type: "category",
    //   label: "Specification",
    //   collapsible: false,
    //   collapsed: false,
    //   items: [{ type: "doc", id: "new/architecture", label: "Architecture" }],
    // },
    // {
    //   type: "category",
    //   label: "Demos",
    //   collapsible: true,
    //   collapsed: true,
    //   items: [
    //     "demos/chatbox-vid",
    //   ],
    // },
    "new/faq",
  ],

  apiSidebar: [
    "api-reference"
  ],

  // communitySidebar: [
  //   "community/support",
  //   "community/contribuiting",
  //   "community/coc",
  //   "community/changelog"
  // ]
};

module.exports = sidebars;
