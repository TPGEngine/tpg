#!/bin/bash

# Enable error handling
set -ex

# Parse arguments to check for -o flag
ENABLE_OPTIMIZATION=0
while getopts "o" opt; do
  case $opt in
    o)
      ENABLE_OPTIMIZATION=1
      ;;
    *)
      # Do nothing for other flags
      ;;
  esac
done


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
if [ -d "$TPG" ]; then
    cd $TPG
else
    echo "TPG directory not found"
    exit 1
fi

# Clean up any existing build
rm -rf build

# Build TPG with or without optimization based on argument
if [ $ENABLE_OPTIMIZATION -eq 1 ]; then
    cmake -B build -S . -DCMAKE_BUILD_TYPE=Release -DENABLE_HIGH_OPTIMIZATION=ON
else
    cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
fi
cmake --build build --config Release
