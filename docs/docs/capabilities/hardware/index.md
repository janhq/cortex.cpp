---
title: Hardware Awareness
draft: True
---

# Hardware Awareness

Cortex is designed to be hardware aware, meaning it can detect your hardware configuration and automatically set parameters to optimize compatibility and performance, and avoid hardware-related errors.

## Hardware Optimization

Cortex's Hardware awareness allows it to do the following: 

- Context Length Optimization: Cortex maximizes the context length allowed by your hardware, ensuring that you can work with larger datasets and more complex models without performance degradation.
- Engine Optimization: we detect your CPU and GPU, and maintain a list of optimized engines for each hardware configuration, e.g. taking advantage of AVX-2 and AVX-512 instructions on CPUs. 

## Hardware Awareness

- Preventing hardware-related error
- Error Handling for Insufficient VRAM: When loading a second model, Cortex provides useful error messages if there is insufficient VRAM memory. This proactive approach helps prevent out-of-memory errors and guides users on how to resolve the issue.

### Model Compatibility

- Model Compatibility Detection: Cortex automatically detects your hardware configuration to determine the compatibility of different models. This ensures that the models you use are optimized for your specific hardware setup.
- This is for the Hub, and for existing Models 

## Hardware Management

### Activating Specific GPUs

Cortex gives you the ability to activating specific GPUs for inference, giving you fine-grained control over hardware resources. This is especially useful for multi-GPU systems. 
- Activate GPUs: Cortex can activate and utilize GPUs to accelerate processing, ensuring that computationally intensive tasks are handled efficiently.
You also have the option to deactivate all GPUs, to run inference on only CPU and RAM. 

### Hardware Monitoring

- Monitoring System Usage
- Monitor VRAM Usage: Cortex keeps track of VRAM usage to prevent out-of-memory (OOM) errors. It ensures that VRAM is used efficiently and provides warnings when resources are running low.
- Monitor System Resource Usage: Cortex continuously monitors the usage of system resources, including CPU, RAM, and GPUs. This helps in maintaining optimal performance and identifying potential bottlenecks.
