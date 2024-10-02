FROM mcr.microsoft.com/devcontainers/cpp:1-ubuntu-22.04

ARG REINSTALL_CMAKE_VERSION_FROM_SOURCE="3.21.5"

# Optionally install the cmake for vcpkg
COPY .devcontainer/reinstall-cmake.sh /tmp/

RUN if [ "${REINSTALL_CMAKE_VERSION_FROM_SOURCE}" != "none" ]; then \
        chmod +x /tmp/reinstall-cmake.sh && /tmp/reinstall-cmake.sh ${REINSTALL_CMAKE_VERSION_FROM_SOURCE}; \
    fi \
    && rm -f /tmp/reinstall-cmake.sh

# Copy requirements.txt to the container
COPY src/tpg/requirements.txt /tmp/requirements.txt

# Update and install packages
RUN apt-get update && \
    xargs -a /tmp/requirements.txt apt-get install -y && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# Set the environment variables
ENV TPG=/workspaces/tpg/src/tpg
ENV PATH=$PATH:$TPG/scripts/plot:$TPG/scripts/run

# [Optional] Uncomment this section to install additional vcpkg ports.
# RUN su vscode -c "${VCPKG_ROOT}/vcpkg install <your-port-name-here>"

# [Optional] Uncomment this section to install additional packages.
# RUN apt-get update && export DEBIAN_FRONTEND=noninteractive \
#     && apt-get -y install --no-install-recommends <your-package-list-here>
