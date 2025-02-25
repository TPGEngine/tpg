# Tangled Program Graphs (TPG)

This code reproduces results from the paper:

Stephen Kelly, Tatiana Voegerl, Wolfgang Banzhaf, and Cedric Gondro. Evolving Hierarchical Memory-Prediction Machines in Multi-Task Reinforcement Learning. Genetic Programming and Evolvable Machines, 2021. [pdf](https://rdcu.be/czd3s)

## Quick Start

This code is designed to be used in Linux. If you use Windows, you can use Windows Subsystem for Linux (WSL). You can work with WSL in Visual Studio Code by following [this tutorial](https://code.visualstudio.com/docs/remote/wsl-tutorial). Run this to automatically install all dependencies and compile:

```bash
bash ./setup.sh
```

This performs the setup and compilation of the steps below. If you want to manually install, follow the instructions below.

For MacOS or Windows users, you can follow this [guide](https://gitlab.cas.mcmaster.ca/kellys32/tpg/-/wikis/Dev-Container-Setup-Guide) to setup Dev Containers which spins up a Linux based environment right within VS Code.

### 1. Install required software

From the tpg directory run:

```
sudo xargs --arg-file requirements.txt apt install
```

Note that [MuJoco](https://mujoco.org/) must be downloaded and unpacked separately.

### 2. Set environment variables

In order to easily access tpg scripts, we add appropriate folders to the $PATH environment variable.
To do so, add the following to _~/.profile_

```
export TPG=<YOUR_PATH_HERE>/tpg
export PATH=$PATH:$TPG/scripts/plot
export PATH=$PATH:$TPG/scripts/run
export MUJOCO=<YOUR_PATH_TO_MUJOCO>/mujoco-3.2.2
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$MUJOCO/lib/
```

Then run:

```
source ~/.profile
```

### 3. Compile

From the tpg directory run:

```bash
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

To run in debug mode:

```bash
cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

To run the build with compiler optimization flags:

```bash
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release -DENABLE_HIGH_OPTIMIZATION=ON
cmake --build build
```

### 4. Run an experiment

Refer to the [wiki](https://gitlab.cas.mcmaster.ca/kellys32/tpg/-/wikis/Running-Experiments-with-the-TPG-CLI) for more information on how to run experiments with the CLI

To run an experiment for [Mujoco Inverted Pendulum](https://gymnasium.farama.org/environments/mujoco/inverted_pendulum/) using 4 parallel MPI processes, we can use the `tpg` CLI tool to execute experiments:

```
tpg evolve inverted_pendulum
```

The logs, plots, and replay videos would populate within the `experiments/inverted_pendulum` directory

Note that as of right now, the number of assigned processes must be greater than the number of active tasks.

### 5. Plot results

Generate classic_control_p0.pdf with various statistics:

```
tpg-plot-stats.sh
```

The first page will be a training curve looking something like the plot below. A fitness of ~1000 indicates the agent balances the pole for 1000 timesteps, thus solving the task.

<img src="./images/MuJoco_Inverted_Pendulum_Fitness.png" height="300" />

### 6. Visualize the best policy's behaviour

Display an OpenGL animation of the single best policy interacting with the environment:

```
tpg replay inverted_pendulum
```

### 7. Cleanup

Delete all checkpoints and output files:

```
tpg-cleanup.sh
```
