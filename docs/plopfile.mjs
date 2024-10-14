import chalk from "chalk";

const capitalize = (str) => {
  return str.charAt(0).toUpperCase() + str.slice(1);
};

const camelCase = (str) => {
  return str.replace(/[-_](\w)/g, (_, c) => c.toUpperCase());
};

/**
 * @param {import("plop").NodePlopAPI} plop
 */
export default async function (plop) {
  plop.setHelper("capitalize", (text) => {
    return capitalize(camelCase(text));
  });

  plop.load("plop-helper-date");

  plop.setGenerator("create-changelog", {
    description: "Generates a changelog",
    prompts: [
      {
        type: "input",
        name: "title",
        message: "Enter the title of the changelog post:",
        validate: (input) => (input ? true : "Title is required."),
      },
      {
        type: "input",
        name: "slug",
        message: (answers) =>
          `Enter the slug for the changelog post (suggested: ${generateSlug(
            answers.title
          )})`,
        default: (answers) => generateSlug(answers.title),
        validate: (input) =>
          input && /^[a-z0-9]+(?:-[a-z0-9]+)*$/.test(input)
            ? true
            : "Please enter a valid slug (lowercase letters, numbers, and hyphens only).",
      },
      {
        type: "input",
        name: "version",
        message: "Enter the version of the changelog post:",
        validate: (input) => (input ? true : "Title is required."),
      },
      {
        type: "input",
        name: "description",
        message: "Enter the description of the changelog post:",
        validate: (input) => (input ? true : "Description is required."),
      },
    ],

    actions(answers) {
      const actions = [];
      if (!answers) return actions;
      const { version, title, description, slug } = answers;

      actions.push({
        type: "addMany",
        templateFiles: "templates/**",
        destination: `./changelog`,
        globOptions: { dot: true },
        data: { title, description, version },
        abortOnFail: true,
      });

      console.log(chalk.green(`Your changelog post is created!`));
      console.log(chalk.green(`You can modify under /changelog/${slug}`));
      console.log(
        chalk.cyan(
          `You can view it at: http://localhost:3000/changelog/${slug}`
        )
      );

      return actions;
    },
  });

  function generateSlug(title) {
    return title
      ? title
          .toLowerCase()
          .replace(/[^a-z0-9]+/g, "-")
          .replace(/^-+|-+$/g, "")
      : "";
  }
}
