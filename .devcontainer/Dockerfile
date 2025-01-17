FROM mcr.microsoft.com/devcontainers/base:ubuntu-24.04

ARG REINSTALL_CMAKE_VERSION_FROM_SOURCE="none"

# Optionally install the cmake for vcpkg
COPY ./reinstall-cmake.sh /tmp/

RUN if [ "${REINSTALL_CMAKE_VERSION_FROM_SOURCE}" != "none" ]; then \
        chmod +x /tmp/reinstall-cmake.sh && /tmp/reinstall-cmake.sh ${REINSTALL_CMAKE_VERSION_FROM_SOURCE}; \
    fi \
    && rm -f /tmp/reinstall-cmake.sh

# Install packages for X11 forwarding
RUN apt-get update && apt-get install -y x11-apps mesa-utils

# Setting XDG_RUNTIME_DIR env variable to store runtime files
ENV XDG_RUNTIME_DIR=/tmp/runtime-dir
RUN mkdir -p /tmp/runtime-dir && chmod 700 /tmp/runtime-dir

# Make sure OpenGL uses indirect rendering
ENV LIBGL_ALWAYS_INDIRECT=1

