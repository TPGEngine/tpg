import glob
import os
import subprocess

import click

from . import helpers


@click.command(help="Evolve a policy")
@click.argument("env")
@click.option("--processes", help="Number of processes to use", default=4)
@click.option("--seed", help="Random seed", default=42)
@click.pass_context
def evolve(ctx: click.Context, env: str, processes: int, seed: int):
    """Evolve a policy for the given environment"""

    # Fetch the hyperparameters for the environment
    hyper_parameters = ctx.obj["hyper_parameters"]
    TPG = ctx.obj["tpg"]

    # error handling for valid environment
    if env not in hyper_parameters:
        raise click.ClickException(f"Environment {env} is not supported. Supported environments are: {', '.join(hyper_parameters.keys())}")
    
    # Setup environment directories and get working directory
    env_dir = helpers.create_environment_directories(TPG, env)
    
    # Change working directory to environment directory
    os.chdir(env_dir)

    # Build the TPGExperimentMPI command
    executable = os.path.join(TPG, "build", "release", "experiments", "TPGExperimentMPI")

    cmd = [
        "mpirun", 
        "--oversubscribe",
        "-np", str(processes),
        executable,
        f"parameters_file={hyper_parameters[env]}",
        f"seed_tpg={seed}",
        f"pid={os.getpid()}"
    ]

    stdout_file = f"tpg.{seed}.{os.getpid()}.std"
    stderr_file = f"tpg.{seed}.{os.getpid()}.err"

    click.echo(f"Launching MPI run with command:\n{' '.join(cmd)}")
    click.echo(f"Output will be written to {stdout_file} (stdout) and {stderr_file} (stderr).")

    try:
        stdout = open(stdout_file, 'w')
        stderr = open(stderr_file, 'w')
        process = subprocess.Popen(cmd, stdout=stdout, stderr=stderr)
        click.echo(f"Process started in the background with PID: {process.pid}")
    except OSError as e:
        raise click.ClickException(f"Failed to start experiment: {e}")

    click.echo("Evolve (started in background)")

@click.command(help="Plot results for an experiment")
@click.argument("env")
@click.argument("csv_files", required=False)
@click.argument("column_name", required=True)
@click.pass_context
def plot(ctx: click.Context, env: str, csv_files: str, column_name: str):
    """Plot results for an experiment using tpg-plot-evolve.py"""

    # Fetch TPG directory for the environment
    hyper_parameters = ctx.obj["hyper_parameters"]
    TPG = ctx.obj["tpg"]

    # error handling for valid environment
    if env not in hyper_parameters:
        raise click.ClickException(f"Environment {env} is not supported. Supported environments are: {', '.join(hyper_parameters.keys())}")

    # Setup environment directories and get working directory
    env_dir = helpers.create_environment_directories(TPG, env)
    
    # Change working directory to environment directory
    os.chdir(env_dir)

    # Build the `tpg-plot-evolve.py`` command
    plot_script_path = os.path.join(TPG, "scripts", "plot", "tpg-plot-evolve.py")

    if not os.path.exists(plot_script_path):
        raise click.ClickException(f"{plot_script_path} does not exist. Ensure the script is located correctly.")

    # Setup environment directories and get working directory
    env_dir = helpers.create_environment_directories(TPG, env)
    
    # Change working directory to environment directory
    os.chdir(env_dir)

    cmd = ["python3", plot_script_path, csv_files, column_name]

    try:
        subprocess.run(cmd, check=True)
    except subprocess.CalledProcessError as e:
        raise click.ClickException(f"Plotting failed: {e}")

@click.command(help="Replay the best performing policy")
@click.argument("env")
@click.option("--seed", help="Random seed", default=42)
@click.option("--seed-aux", help="Auxillary seed", default=42)
@click.option("--task-to-replay", help="Task to replay for multitask", default=0)
@click.pass_context
def replay(ctx: click.Context, env: str, seed: int, seed_aux: int, task_to_replay: int):
    """Replay the best performing policy for the given environment"""
    
    # Fetch the hyperparameters for the environment
    hyper_parameters = ctx.obj["hyper_parameters"]
    TPG = ctx.obj["tpg"]
    
    env_dir = helpers.create_environment_directories(TPG, env)
    
    # Change working directory to environment directory
    os.chdir(env_dir)

    # Find the selection.*.*.csv file
    csv_files = glob.glob(os.path.join(env_dir, "selection.*.*.csv"))
    if not csv_files:
        raise click.ClickException("Ensure that you've evolved a policy before replaying it and the selection.*.*.csv file exists.")

    csv_file = csv_files[0] 

    # get the best fitness, generation and team id
    metrics = helpers.get_metrics_from_csv(csv_file)

    # get the checkpoint values
    checkpoint_in_phase, checkpoint_in_t = helpers.get_checkpoint_values(seed)

    # build replay command
    executable = os.path.join(TPG, "build", "release", "experiments", "TPGExperimentMPI")

    cmd = [
        "mpirun", 
        "--oversubscribe",
        "-np", str(1),
        executable,
        f"parameters_file={hyper_parameters[env]}",
        f"seed_tpg={seed}",
        f"seed_aux={seed_aux}",
        f"start_from_checkpoint=1",
        f"checkpoint_in_phase={checkpoint_in_phase}",
        f"checkpoint_in_t={checkpoint_in_t}",
        f"replay=1", 
        f"animate=1",
        f"id_to_replay={int(metrics['team_id'])}",
        f"task_to_replay={task_to_replay}"
    ]

    stdout_file = f"tpg.{seed}.{seed_aux}.replay.std"
    stderr_file = f"tpg.{seed}.{seed_aux}.replay.err"

    click.echo(f"Fitness: {metrics['best_fitness']}, Generation: {metrics['generation']}, Team ID: {metrics['team_id']}")
    
    click.echo(f"Launching MPI run with command:\n{' '.join(cmd)}")
    click.echo(f"Output will be written to {stdout_file} (stdout) and {stderr_file} (stderr).")

    try:
        stdout = open(stdout_file, 'w')
        stderr = open(stderr_file, 'w')
        process = subprocess.Popen(cmd, stdout=stdout, stderr=stderr)
        click.echo(f"Process started in the background with PID: {process.pid}")
    except OSError as e:
        raise click.ClickException(f"Failed to start experiment: {e}")

    click.echo("Replay (started in background)")    
