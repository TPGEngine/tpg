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

# Build Python CLI
RUN pipx ensurepath && \
    cd src/cli && \
    pipx install -e .

# api stage
FROM python:3.12-slim AS api

WORKDIR /app

# Copy over CLI binaries and build artifacts from the builder stage
COPY --from=builder /root/.local/bin /root/.local/bin
COPY --from=builder /usr/local/lib/ /usr/local/lib/
COPY --from=builder /tpg/build/ /app/build
COPY --from=builder /tpg/configs/ /app/configs
COPY --from=builder /tpg/src/cli/ /app/cli
COPY --from=builder /tpg/src/api/ /app/api

# Create non-root user
RUN groupadd -r appuser && useradd -r -g appuser -m appuser

RUN mkdir -p /app/experiments

# Install mpi
RUN apt-get update && apt-get install -y mpi

# Install pipx in this stage so we have the pipx command available
RUN pip install pipx && pipx ensurepath

ENV PATH="/root/.local/bin:${PATH}"

WORKDIR /app/cli

RUN pipx install -e .

WORKDIR /app/api

ENV TPG=/tpg \
    MUJOCO=/usr/local/lib/mujoco-3.2.2 \
    LD_LIBRARY_PATH="/usr/local/lib/mujoco-3.2.2/lib" \
    PATH="${PATH}:/tpg/scripts/plot:/tpg/scripts/run"

# Now install the uv CLI and sync
RUN pipx install uv && uv sync

# Set proper ownership
RUN chown -R appuser:appuser /app /root/.local

# Switch to non-root user
USER appuser

CMD ["/app/api/.venv/bin/fastapi", "run", "app/main.py", "--port", "80", "--host", "0.0.0.0"]

    