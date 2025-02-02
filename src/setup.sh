r#!/bin/bash

# Update apt-get repository
sudo apt-get update

# Install Packages
sudo xargs --arg-file requirements.txt apt install -y

# Install MuJoCo
cd /usr/local/lib

# Determine the architecture
ARCH=$(uname -m)

if [[ "$ARCH" == "x86_64" ]]; then
    MUJOCO_URL="https://github.com/deepmind/mujoco/releases/download/3.2.2/mujoco-3.2.2-linux-x86_64.tar.gz"
    git config core.autocrlf false
    git rm --cached -r .
    git reset --hard 
elif [[ "$ARCH" == "aarch64" ]]; then
    MUJOCO_URL="https://github.com/deepmind/mujoco/releases/download/3.2.2/mujoco-3.2.2-linux-aarch64.tar.gz"
else
    echo "Unsupported architecture: $ARCH"
    exit 1
fi

sudo wget "$MUJOCO_URL"
MUJOCO_TAR=$(basename "$MUJOCO_URL")
sudo tar -xzf "$MUJOCO_TAR"

# Change directory to TPG source
cd /workspaces/tpg

# Build TPG
scons --opt