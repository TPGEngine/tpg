#!/bin/bash

# Install Packages
sudo xargs --arg-file requirements.txt apt install -y

# Install MuJoCo
cd /usr/local/lib
sudo wget https://github.com/deepmind/mujoco/releases/download/3.2.2/mujoco-3.2.2-linux-aarch64.tar.gz
sudo tar -xzf mujoco-3.2.2-linux-aarch64.tar.gz

# Set environment variables
export TPG=/workspaces/tpg/src
export PATH=$PATH:$TPG/scripts/plot
export PATH=$PATH:$TPG/scripts/run
export MUJOCO=/usr/local/lib/mujoco-3.2.2
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$MUJOCO/lib/

source ~/.profile

# Change directory to TPG source
cd /workspaces/tpg/src

# Build TPG
scons --opt