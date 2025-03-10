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
- **Plot Results:** Plot statistics about experiments.

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
tpg evolve <env>
```

This command will:

Verify that the given <env> is supported (as defined in your hyper_parameters).
Build a command to launch the TPG experiment using mpirun with the specified number of processes.
Redirect logs about the experiment to the `$TPG/experiments/<env>/logs` directory. There are subdirectories for each stage of the evolutionary training process: `replacement`, `removal`, `selection`, and `timing`.

**Arguments:**

- env (str): The target environment

**Options:**

- (-s) seed (int): Specifying a specific random number to run
- (-p) processes (int): Specifying the number of parallel processes to run, this value defaults to 4
- (-n) num-experiments (int): Specifying the number of experiments to executes

Here are the environments currently supported:

- classic_control
- half_cheetah
- hopper
- humanoid_standup
- inverted_pendulum
- inverted_double_pendulum
- multitask
- multitask_half_cheetah

**Note:** The environments supported are related to the yaml files present within `$TPG/src/configs`. The naming convention is extracting the string after `MuJoco_` and making everything lowercase and snakecase.

**Example:**
Running 3 experiemnts of the `inverted_pendulum` environment with 3 different seeds.

```bash
tpg evolve inverted_pendulum -n 3
```

**Plot Results from an Environment**

```bash
tpg plot <env> <csv_files> <column_name>
```

This plotting command will:

Generate a plot based on the CSV data provided from a previous evolution run.
Utilize the specified column from the CSV for plotting.

**Arguments:**
env (str): The target environment.
csv_files (str): The path to the CSV files containing the results.
column_name (str): The name of the column to plot.

For more information regarding arguments `csv_files` and `column_name`, visit our [Wiki](https://gitlab.cas.mcmaster.ca/kellys32/tpg/-/wikis/TPG-Generation-Plot-for-CSV-Logging-Files).

**Example:**

```bash
tpg plot half_cheetah all-timing generation_time
```

**Options:**

- --processes (int): Number of processes to use (default: 4).
- --seed (int): Random seed (default: 42).

**Replay the Best Policy:**

```bash
tpg replay <env>
```

The replay command:

Scans for a selection._._.csv file (generated from a previous evolve run).
Uses helper functions to extract the best performing team's ID and checkpoint information.
Launches an MPI run in replay mode with the given parameters. If running in a Dev Containers environment, MP4 replays would be populated within the `$TPG/experiments/<env>/videos` directory.

**Arguments:**

- env (str): The target environment

**Options:**

- (-s) seed (int): Replaying a specific seed for that environment
- (--seed-aux) auxillary seed (int): An auxiliary seed (default: 42) used in the replay; its exact role is determined by the underlying experiment logic.
- (-t) task to replay (int): Option for multitask experiments which task to visualize

**Example:**

```bash
tpg replay inverted_pendulum -s 2
```

This command would replay the best policy from a previously evolved experiment from a seed with value 2.

**Cleanup the Experiment Directory:**

```bash
tpg clean <env>
```

The clean command:

Removes the directory associated with the specified environment.

**Arguments:**

- env (str): The target environment

**Example:**

```bash
tpg clean inverted_pendulum
```

**Kill the evolution of an experiment**

```bash
tpg kill <env>
```

The clean command:

Kills the processes running the specified environment, or kills all running processes if no environment is specified.

**Arguments:**

- env (str): The target environment (optional; will kill all experiment processes if no environment is specified)

**Example:**

```bash
tpg kill inverted_pendulum
```

## Troubleshooting

If attempting to execute a tpg command leads you to an error such as: `Command 'tpg' not found`. Ensure your PATH
environment variables are up to date. You can do this by running `pipx ensurepath` and restarting your terminal.
