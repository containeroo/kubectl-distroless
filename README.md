# Minimal Kubectl Docker Image

This repository provides a minimal Docker image for executing `kubectl` commands, built using the `gcr.io/distroless/static-debian12` base image for maximum security and minimal size.

## Features

- Based on Google's Distroless `static-debian12` image.
- Includes `kubectl` for Kubernetes cluster management.
- Small and secure, with no unnecessary tools or packages.
- Pre-installed CA certificates for HTTPS connections.

