import type { JestConfigWithTsJest } from "ts-jest";

const jestConfig: JestConfigWithTsJest = {
  preset: "ts-jest",
  testEnvironment: "node",
  transformIgnorePatterns: ["/node_modules/"],
  globals: {
    RELEASE_URL_PREFIX: "https://api.github.com/repos/janhq/nitro/releases/",
    TAGGED_RELEASE_URL_PREFIX:
      "https://api.github.com/repos/janhq/nitro/releases/tags",
  },
};

export default jestConfig;
