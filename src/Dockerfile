# builder stage 
FROM ubuntu:24.04 AS builder

WORKDIR /tpg

COPY . .

RUN apt-get update 

RUN xargs --arg-file=requirements.txt apt install -y 

# Install MuJoCo
WORKDIR /usr/local/lib
RUN ARCH=$(uname -m) && \
    if [ "$ARCH" = "x86_64" ]; then \
        MUJOCO_URL="https://github.com/deepmind/mujoco/releases/download/3.2.2/mujoco-3.2.2-linux-x86_64.tar.gz"; \
    elif [ "$ARCH" = "aarch64" ]; then \
        MUJOCO_URL="https://github.com/deepmind/mujoco/releases/download/3.2.2/mujoco-3.2.2-linux-aarch64.tar.gz"; \
    else \
        echo "Unsupported architecture: $ARCH"; \
        exit 1; \
    fi && \
    wget ${MUJOCO_URL} && \
    MUJOCO_TAR=$(basename ${MUJOCO_URL}) && \
    tar -xzf ${MUJOCO_TAR}

ENV TPG=/tpg \
    MUJOCO=/usr/local/lib/mujoco-3.2.2 \
    LD_LIBRARY_PATH=/usr/local/lib/mujoco-3.2.2/lib \
    PATH="${PATH}:/tpg/scripts/plot:/tpg/scripts/run"

# Build TPG 
WORKDIR /tpg
RUN rm -rf build && \
    make optimized 

# api stage
FROM ubuntu:24.04 AS api

# install TPGExperimentMPI dependencies
RUN apt-get update && apt-get install -y \
    python3-pip \
    pipx \
    libboost-all-dev \
    libcurl4-openssl-dev \
    libglfw3-dev \
    libtbb-dev \
    freeglut3-dev \
    libosmesa6-dev \
    libyaml-cpp-dev \
    libglx-dev \
    openmpi-bin \
    libopenmpi-dev

# Create non-root user
RUN groupadd -r appuser && useradd -r -g appuser -m appuser

WORKDIR /app

# Copy over CLI binaries and build artifacts from the builder stage
COPY --from=builder /usr/local/lib/ /usr/local/lib/
COPY --from=builder /tpg/build/ /app/build
COPY --from=builder /tpg/configs/ /app/configs
COPY --from=builder /tpg/src/cli/ /app/cli
COPY --from=builder /tpg/src/api/ /app/api

# Create directories and set permissions
RUN mkdir -p /app/experiments && \
    chown -R appuser:appuser /app

# ONLY NOW switch to non-root user
USER appuser

ENV TPG=/app \
    MUJOCO=/usr/local/lib/mujoco-3.2.2 \
    LD_LIBRARY_PATH="/usr/local/lib/mujoco-3.2.2/lib" \
    PATH="/home/appuser/.local/bin:${PATH}:/tpg/scripts/plot:/tpg/scripts/run"

# Install user tools as non-root
# No need to pip install pipx now - just ensure it's in path
RUN pipx ensurepath && \
    echo 'export PATH="$PATH:$HOME/.local/bin"' >> ~/.bashrc

# Continue with user-level installations
WORKDIR /app/cli
RUN pipx install -e .

WORKDIR /app/api
RUN pipx install uv && \
    uv sync

CMD ["/app/api/.venv/bin/fastapi", "run", "app/main.py", "--port", "8080", "--host", "0.0.0.0"]