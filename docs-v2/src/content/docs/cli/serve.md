---
title: Cortex Serve
description: Cortex serve command.
slug: "serve"
---

:::warning
🚧 Cortex.cpp is currently under development. Our documentation outlines the intended behavior of Cortex, which may not yet be fully implemented in the codebase.
:::

# `cortex serve`

This command runs the API endpoint server for the Cortex back-end.



## Usage

```bash
# Start the API server
cortex serve [options]
# Stop the API server
cortex serve [options] stop
```

## Options

| Option                     | Description                               | Required | Default Value | Example                |
|----------------------------|-------------------------------------------|----------|---------------|------------------------|
| `-a`, `--address <address>`  | Specify the address to use.               | No       | `localhost`   | `-a 192.168.1.1`|
| `-p`, `--port <port>`        | Define the port to serve the application. | No       | `3928`        | `-p 8080`          |
| `-d`, `--detach`             | Run the server in detached mode.          | No       | `false`       | `-d`             |
| `-h`, `--help`               | Display help information for the command. | No       | -             | `-h`               |


