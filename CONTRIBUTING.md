# Contributing to jan

First off, thank you for considering contributing to Jan Nitro.

## How Can I Contribute?

### Reporting Bugs

- **Ensure the bug was not already reported** by searching on GitHub under [Issues](https://github.com/janhq/nitro/issues).
- If you're unable to find an open issue addressing the problem, [open a new one](https://github.com/janhq/nitro/issues/new).

### Suggesting Enhancements

- Open a new issue with a clear title and description.

### Your First Code Contribution

You can start to develop on this repo using the pre-packaged docker container with clangd (language server for c++) already set up. Inside the docker container also has a neovim config that has features like code-completion, filetree,etc...

In order to get started developing, just 2 steps

#### Step 1: build development docker image
Fork the repo run the build development environment code.
```zsh
./dev_build.sh
```
This will build a container image named "nitro_dev_env"
#### Step 2: run the development container and start developing
```zsh
docker run --name dev_nitro -t -d nitro_dev_env 
```
Then you can `docker attach dev_nitro`, and inside the container if you wish to use neovim to develop
#### Step 3 (optional): use neovim
Develop with fully functional neovim
```zsh
./scripts/dev_entrypoint.sh
```

### How to compile the source code ?

As of now it is not possible to compile the code outside of the container we built above, but if you want change the code and re-compile it yourself, you can attach to the container that we built and run above and do the compilation steps in [this folder](inference_backend)

### How the backend is implemented
Information about how some parts of the backend is implemented can be found at [Developer Documentation](docs/development)

## Styleguides

### Git Commit Messages

- Use the present tense ("Add feature" not "Added feature").

## Additional Notes

Thank you for contributing to Jan!
