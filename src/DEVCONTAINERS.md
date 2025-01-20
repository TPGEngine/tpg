# Dev Container Onboarding Guide

This guide will help you get started with the TPG developer environment using Dev Containers which spins up a reproducible environment for those who aren't on Linux

## Prerequisites

Before you begin, make sure you have the following installed on your system:

1. Docker
2. VS Code
3. Dev Containers VS Code Extension

## Setting up Docker

1. Download and install Docker for your operating system:
   - [Docker Desktop for Windows](https://docs.docker.com/docker-for-windows/install/)
   - [Docker Desktop for Mac](https://docs.docker.com/docker-for-mac/install/)

2. Run the following command to ensure Docker is running properly
```
docker --version
```

## Activating the Dev Container in VS Code

1. Open TPG Visual Studio Code.

2. When VS Code detects the Dev Container configuration, you'll see a 
notification in the bottom-right corner. Click on "Reopen in Container" to 
start the Dev Container.

Alternatively, you can:
- Press `CMD + SHIFT + P` on Mac or `CTRL + SHIFT + P` on Windows to open the command palette
- Type `Dev Containers: Reopen in Container` and select it

3. VS Code will now build and start the Dev Container. This may take a few 
minutes the first time.

4. Once the Dev Container is ready, you'll see the VS Code window reload, and 
you'll be working inside the container environment.

## Benefits of Using Dev Containers

Our Dev Container provides a consistent, pre-configured development environment 
with the following advantages:

- All necessary dependencies and tools are pre-installed
- Consistent development environment across different machines and operating 
systems
- Easy onboarding for new team members
- Isolated environment that doesn't affect your local system
