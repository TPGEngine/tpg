# TPG CLI Policy Evolution and Replay

This project provides a Python command-line interface (CLI) powered by
[Click](https://click.palletsprojects.com/) to manage experiments for evolving,
plotting, and replaying policies. The tool leverages MPI through `mpirun` to execute
the underlying experiments defined in the TPG framework.

> **Note:** This CLI tool expects that you have the TPG binaries built (specifically,
> the `TPGExperimentMPI` executable) and that you provide a configuration context
> (via `ctx.obj`) containing appropriate values for:
>
> - **`hyper_parameters`:** A dictionary mapping supported environment names to their
>   respective parameter files.
> - **`tpg`:** The $TPG environment variable that is the root of the TPG codebase

---

## Features

- **Evolve a Policy:** Launch an MPI run to evolve a policy for a specified environment.
- **Replay a Policy:** Replay the best performing policy based on experiment outputs.
- **Plot Results (Work in Progress):** A placeholder for future plotting functionality.

---

## Installation

Ensure that you have `pipx` installed:
```bash
sudo apt install pipx
```

To utilize this CLI tool anywhere in the repo:
```bash
cd $TPG/src/cli
pipx install --editable .
```

**Note:** The `--editable` flag allows this package to be editable. We can modify the source code and have those changes immediately installed without having to reinstall the package after making a change.

## Usage

Here are the main commands.

**Evolve a Policy:**
```bash
cd $TPG/experiments/generic
tpg evolve <env> --processes 4
```
This command will:

Verify that the given <env> is supported (as defined in your hyper_parameters).
Build a command to launch the TPG experiment using mpirun with the specified number of processes.
Redirect standard output and error to files named in the format:
- tpg.<seed>.<pid>.std
- tpg.<seed>.<pid>.err

**Arguments:**
- env (str): The target environment

Here are the environments currently supported:
- classic_control
- half_cheetah
- hopper
- humanoid_standup
- inverted_pendulum
- inverted_double_pendulum
- multitask
- multitask_half_cheetah

**Options:**
- --processes (int): Number of processes to use (default: 4).
- --seed (int): Random seed (default: 42).

**Replay the Best Policy:**

```bash
tpg replay <env> 
```
The replay command:

Scans for a selection.*.*.csv file (generated from a previous evolve run).
Uses helper functions to extract the best performing team's ID and checkpoint information.
Launches an MPI run in replay mode with the given parameters.
Writes the output to files:
- tpg.<seed>.<seed_aux>.replay.std
- tpg.<seed>.<seed_aux>.replay.err

**Arguments:**
- env (str): The target environment
**Options:**
- --seed (int): Random seed for the replay (default: 42).
- --seed-aux (int): An auxiliary seed (default: 42) used in the replay; its exact role is determined by the underlying experiment logic.
- --task-to-replay (int): Option for multitask experiments which task to visualize
j
## Troubleshooting
If attempting to execute a tpg command leads you to an error such as: `Command 'tpg' not found`. Ensure your PATH
environment variables are up to date. You can do this by running `pipx ensurepath` and restarting your terminal. 