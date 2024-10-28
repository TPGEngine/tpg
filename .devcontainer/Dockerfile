FROM ubuntu:latest

# Copy requirements.txt to the container
COPY src/requirements.txt /tmp/requirements.txt

# Install C++ build tools and packages from requirements.txt
RUN apt-get update && \
    apt-get install -y build-essential gdb cmake libglfw3-dev curl && \
    xargs -a /tmp/requirements.txt apt-get install -y && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# Install MuJoCo
RUN apt-get update && apt-get install -y wget && \
    wget https://github.com/deepmind/mujoco/releases/download/3.2.4/mujoco-3.2.4-linux-x86_64.tar.gz && \
    mkdir -p /usr/local/lib/mujoco && \
    tar -xf mujoco-3.2.4-linux-x86_64.tar.gz -C /usr/local/lib/mujoco && \
    rm mujoco-3.2.4-linux-x86_64.tar.gz && \
    ln -s /usr/local/lib/mujoco/mujoco-3.2.4/include/mujoco /usr/local/include/mujoco && \
    ln -s /usr/local/lib/mujoco/mujoco-3.2.4/lib/libmujoco.so.3.2.4 /usr/local/lib/libmujoco.so

# Install GLEW
RUN apt-get install -y libglew-dev

# Set environment variables
ENV TPG=/workspaces/tpg/src
ENV PATH="${PATH}:${TPG}/scripts/plot:${TPG}/scripts/run"
ENV LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib/mujoco/mujoco-3.2.4/lib
ENV MUJOCO_PATH=/usr/local/lib/mujoco/mujoco-3.2.4
