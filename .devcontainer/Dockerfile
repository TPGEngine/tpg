FROM mcr.microsoft.com/devcontainers/base:ubuntu-24.04

ARG REINSTALL_CMAKE_VERSION_FROM_SOURCE="none"

# Optionally install the cmake for vcpkg
COPY ./reinstall-cmake.sh /tmp/

RUN if [ "${REINSTALL_CMAKE_VERSION_FROM_SOURCE}" != "none" ]; then \
        chmod +x /tmp/reinstall-cmake.sh && /tmp/reinstall-cmake.sh ${REINSTALL_CMAKE_VERSION_FROM_SOURCE}; \
    fi \
    && rm -f /tmp/reinstall-cmake.sh

# Make sure OpenGL uses indirect rendering
ENV LIBGL_ALWAYS_INDIRECT=1

# set ENV variables to access TPG scripts
ENV TPG=/workspaces/tpg/src
ENV MUJOCO=/usr/local/lib/mujoco-3.2.2
ENV PATH="${PATH}:${TPG}/scripts/plot:${TPG}/scripts/run"
ENV LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:${MUJOCO}/lib"
