# Inference Backend Documentation

## Compilation Guide

This document outlines the steps to compile the Inference Backend project. Depending on your requirements, there are two options available for compiling: creating a single binary or generating individual binaries for each controller.

---

### Step 1: Create a Build Directory
Firstly, navigate to the project root directory and create a new folder named `build`. To do this, execute the following commands:

```bash
mkdir build
cd build
```

### Step 2: Choose a Compilation Option

#### Option 1: Compile a Single Binary

To compile the project into a single binary, run the following commands:

```bash
cmake .. -DBUILD_SINGLE_BINARY=true
make
```

#### Option 2: Compile Individual Binaries for Controllers

To compile individual binaries for each controller, execute:

```bash
cmake ..
make
```

This will generate a binary for each controller in the `inference_backend/controllers` directory. As of the last update, there are currently two controllers.

---

By following these steps, you can successfully compile the Inference Backend project according to your specific needs.