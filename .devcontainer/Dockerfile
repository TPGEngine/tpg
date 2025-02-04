FROM mcr.microsoft.com/devcontainers/base:ubuntu-24.04

# Make sure OpenGL uses indirect rendering
ENV LIBGL_ALWAYS_INDIRECT=1

# set ENV variables to access TPG scripts
ENV TPG=/workspaces/tpg/src
ENV MUJOCO=/usr/local/lib/mujoco-3.2.2
ENV PATH="${PATH}:${TPG}/scripts/plot:${TPG}/scripts/run"
ENV LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:${MUJOCO}/lib"
