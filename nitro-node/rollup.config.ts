import resolve from "@rollup/plugin-node-resolve";
import commonjs from "@rollup/plugin-commonjs";
import sourceMaps from "rollup-plugin-sourcemaps";
import typescript from "rollup-plugin-typescript2";
import json from "@rollup/plugin-json";
import replace from "@rollup/plugin-replace";

export default [
  {
    input: `src/index.ts`,
    output: [
      { file: "dist/index.cjs", format: "cjs", sourcemap: true },
      { file: "dist/index.esm.js", format: "es", sourcemap: true },
    ],
    // Indicate here external modules you don't wanna include in your bundle (i.e.: 'lodash')
    external: [
      // `download@8.0.0` requires `got@^8.3.1` which then optionally requires `electron`, result in wrong dependency
      // Ref: https://github.com/kubernetes-client/javascript/issues/350#issue-500860208
      // Ref: https://github.com/kubernetes-client/javascript/issues/350#issuecomment-553644659
      "got",
    ],
    watch: {
      include: "src/**",
    },
    plugins: [
      // Allow json resolution
      json(),
      // Allow node_modules resolution, so you can use 'external' to control
      // which external modules to include in the bundle
      // https://github.com/rollup/rollup-plugin-node-resolve#usage
      resolve({
        extensions: [".ts", ".js", ".json"],
      }),
      // Allow bundling cjs modules (unlike webpack, rollup doesn't understand cjs)
      // This should be after resolve() plugin
      commonjs(),
      // Compile TypeScript files
      typescript({
        useTsconfigDeclarationDir: true,
      }),

      // Resolve source maps to the original source
      sourceMaps(),
    ],
  },
  {
    input: `src/scripts/index.ts`,
    output: [
      {
        file: "dist/scripts/index.cjs",
        format: "cjs",
        sourcemap: true,
      },
      {
        file: "dist/scripts/index.esm.js",
        format: "es",
        sourcemap: true,
      },
    ],
    // Indicate here external modules you don't wanna include in your bundle (i.e.: 'lodash')
    external: [
      // `download@8.0.0` requires `got@^8.3.1` which then optionally requires `electron`, result in wrong dependency
      // Ref: https://github.com/kubernetes-client/javascript/issues/350#issue-500860208
      // Ref: https://github.com/kubernetes-client/javascript/issues/350#issuecomment-553644659
      "got",
    ],
    watch: {
      include: "src/scripts/**",
    },
    plugins: [
      replace({
        RELEASE_URL_PREFIX: JSON.stringify(
          "https://api.github.com/repos/janhq/nitro/releases/",
        ),
        TAGGED_RELEASE_URL_PREFIX: JSON.stringify(
          "https://api.github.com/repos/janhq/nitro/releases/tags",
        ),
      }),
      // Allow json resolution
      json(),

      // Allow node_modules resolution, so you can use 'external' to control
      // which external modules to include in the bundle
      // https://github.com/rollup/rollup-plugin-node-resolve#usage
      resolve({
        extensions: [".ts", ".js", ".json"],
      }),

      // Allow bundling cjs modules (unlike webpack, rollup doesn't understand cjs)
      // This should be after resolve() plugin
      commonjs(),
      // Compile TypeScript files
      typescript({
        useTsconfigDeclarationDir: true,
      }),

      // Resolve source maps to the original source
      sourceMaps(),
    ],
  },
];
