# Website

This website is built using [Docusaurus](https://docusaurus.io/), a modern static website generator.

### Installation

```
$ yarn
```

### Local Development

```
$ yarn start
```

This command starts a local development server and opens up a browser window. Most changes are reflected live without having to restart the server.

### Build

```
$ yarn build
```

This command generates static content into the `build` directory and can be served using any static contents hosting service.

### Deployment

Using SSH:

```
$ USE_SSH=true yarn deploy
```

Not using SSH:

```
$ GIT_USER=<Your GitHub username> yarn deploy
```

If you are using GitHub pages for hosting, this command is a convenient way to build the website and push to the `gh-pages` branch.

## Changelog Generator

To generate a changelog post, run:

```bash
  yarn create:changelog
```

- **Title & Slug**: Generate changelog post files with a title and a slug.
- **Description**: Add a description for the changelog post.
- **Version**: Add a version for the changelog post.

The pages will be generated in `changelog/${slug}`. You can start writing your changelog post here.
