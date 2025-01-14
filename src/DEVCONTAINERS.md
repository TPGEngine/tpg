# Onboarding to Tangled Program Graphs (TPG) Using VSCode Dev Containers

This guide provides step-by-step instructions to onboard to the TPG codebase using VSCode Dev Containers. These instructions are designed to help MacOS users (and other non-Linux users) quickly set up a development environment with Linux dependencies already configured.

---

## Prerequisites

1. **Install Docker**
   - Download and install Docker Desktop from [Docker's official website](https://www.docker.com/products/docker-desktop/).
   - Follow the installation guide for your operating system (e.g., MacOS).
   - Verify the installation by running:
     ```bash
     docker --version
     ```

2. **Install VSCode**
   - Download and install Visual Studio Code from [VSCode's official website](https://code.visualstudio.com/).

3. **Install the Remote - Containers Extension**
   - Open VSCode.
   - Go to the Extensions view by clicking on the Extensions icon in the Activity Bar on the side of the window or by pressing `Ctrl+Shift+X`.
   - Search for `Dev Containers` and install the **Remote - Containers** extension by Microsoft.

## Developing in the Container Environment

**Reopen in Container**
   - Ensure the repository contains a `.devcontainer` folder with the necessary configuration files.
   - If the `.devcontainer` folder exists, VSCode will prompt you with a notification to reopen the folder in the container. Click **Reopen in Container**.
   - Alternatively, open the Command Palette (`Ctrl+Shift+P` or `Cmd+Shift+P` on Mac) and select:
     ```
     Dev Containers: Reopen in Container
     ```

## Using the Dev Container

1. Install MuJoCo v3.2.2
```
wget https://github.com/deepmind/mujoco/releases/download/3.2.2/mujoco-3.2.2-linux-aarch64.tar.gz
tar -xzf mujoco-3.2.2-linux-aarch64.tar.gz
```

Note: Automated environment setup is in WIP, for now refer back to [README.md](./README.md) since your now developing in a standardized Linux environment!