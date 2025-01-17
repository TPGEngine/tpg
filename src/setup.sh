#!/bin/bash

# Install Packages
sudo xargs --arg-file requirements.txt apt install -y

# Install MuJoCo
cd /usr/local/lib

# Determine the architecture
ARCH=$(uname -m)

if [[ "$ARCH" == "x86_64" ]]; then
    MUJOCO_URL="https://github.com/deepmind/mujoco/releases/download/3.2.2/mujoco-3.2.2-linux-x86_64.tar.gz"
elif [[ "$ARCH" == "aarch64" ]]; then
    MUJOCO_URL="https://github.com/deepmind/mujoco/releases/download/3.2.2/mujoco-3.2.2-linux-aarch64.tar.gz"
else
    echo "Unsupported architecture: $ARCH"
    exit 1
fi

sudo wget "$MUJOCO_URL"
MUJOCO_TAR=$(basename "$MUJOCO_URL")
sudo tar -xzf "$MUJOCO_TAR"

# Set environment variables
export TPG=/workspaces/tpg/src
export PATH=$PATH:$TPG/scripts/plot
export PATH=$PATH:$TPG/scripts/run
export MUJOCO=/usr/local/lib/mujoco-3.2.2
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$MUJOCO/lib/

# Persist environment variables
echo "export TPG=$TPG" >> ~/.profile
echo "export PATH=$PATH" >> ~/.profile
echo "export MUJOCO=$MUJOCO" >> ~/.profile
echo "export LD_LIBRARY_PATH=$LD_LIBRARY_PATH" >> ~/.profile

source ~/.profile

# Change directory to TPG source
cd /workspaces/tpg/src

# Build TPG
scons --opt
