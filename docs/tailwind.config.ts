import type { Config } from "tailwindcss";

const config: Config = {
  corePlugins: {
    preflight: false,
    container: false,
  },
  darkMode: ["class", '[data-theme="dark"]'],
  content: [
    "./src/**/*.{jsx,tsx,html,md,scss}",
    "./src/components/**/*.{jsx,tsx,html,md,scss}",
  ],
  theme: {
    container: {
      center: true,
      padding: "16px",
    },
    fontFamily: {
      sans: [
        "Inter",
        "-apple-system",
        "BlinkMacSystemFont",
        "Segoe UI",
        "Roboto",
        "Oxygen-Sans",
        "Ubuntu,Cantarell",
        "Helvetica",
        "sans-serif",
      ],
      grotesk: [
        "Space Grotesk",
        "-apple-system",
        "BlinkMacSystemFont",
        "Segoe UI",
        "Roboto",
        "Oxygen-Sans",
        "Ubuntu,Cantarell",
        "Helvetica",
        "sans-serif",
      ],
    },
    extend: {},
  },
  plugins: [],
};
export default config;
