import glob
import os
import subprocess
import shutil

import click

from . import helpers


@click.command(help="Evolve a policy")
@click.argument("env")
@click.option("-p", "--processes", help="Number of processes to use", default=4)
@click.option("-s", "--seed", help="Random seed", default=42)
@click.option("-n", "--num-experiments", help="Number of experiments to run", default=1)
@click.pass_context
def evolve(ctx: click.Context, env: str, processes: int, seed: int, num_experiments: int):
    """Evolve a policy for the given environment"""

    # Fetch the hyperparameters for the environment
    hyper_parameters = ctx.obj["hyper_parameters"]
    TPG = ctx.obj["tpg"]

    # Error handling for valid environment
    if env not in hyper_parameters:
        raise click.ClickException(
            f"Environment {env} is not supported. Supported environments are: "
            f"{', '.join(hyper_parameters.keys())}"
        )
    
    # Setup environment directories and get working directory
    env_dir = helpers.create_environment_directories(TPG, env)
    
    # Change working directory to environment directory
    os.chdir(env_dir)

    # Create logs directory if it doesn't exist
    os.makedirs('logs', exist_ok=True)

    # Build the TPGExperimentMPI command
    executable = os.path.join(TPG, "build", "release", "experiments", "TPGExperimentMPI")

    # Run the appropriate number of experiments
    if num_experiments > 1:
        # Run multiple experiments with sequential seeds
        for i in range(1, num_experiments + 1):
            helpers.run_single_experiment(
                executable=executable,
                parameters_file=hyper_parameters[env],
                processes=processes,
                seed=i
            )
    else:
        # Run a single experiment with the provided seed
        helpers.run_single_experiment(
            executable=executable,
            parameters_file=hyper_parameters[env],
            processes=processes,
            seed=seed
        )


@click.command(help="Plot results for an experiment")
@click.argument("env", required=True)
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

    # Build the `tpg-plot-evolve.py`` command
    plot_script_path = os.path.join(TPG, "scripts", "plot", "tpg-plot-evolve.py")

    if not os.path.exists(plot_script_path):
        raise click.ClickException(f"{plot_script_path} does not exist. Ensure the script is located correctly.")

    # Setup environment directories and get working directory
    env_dir = helpers.create_environment_directories(TPG, env)
    
    # Change working directory to environment directory
    os.chdir(env_dir)

    # Create plots directory if it doesn't exist
    if not os.path.exists('plots'):
        os.makedirs('plots')

    cmd = ["python3", plot_script_path, csv_files, column_name]

    try:
        subprocess.run(cmd, check=True)
    except subprocess.CalledProcessError as e:
        raise click.ClickException(f"Plotting failed: {e}")

@click.command(help="Replay the best performing policy")
@click.argument("env")
@click.option("-s", "--seed", help="Random seed")
@click.option("--seed-aux", help="Auxillary seed", default=42)
@click.option("-t", "--task-to-replay", help="Task to replay for multitask", default=0)
@click.pass_context
def replay(ctx: click.Context, env: str, seed: int, seed_aux: int, task_to_replay: int):
    """Replay the best performing policy for the given environment"""
    
    # Fetch the hyperparameters for the environment
    hyper_parameters = ctx.obj["hyper_parameters"]
    TPG = ctx.obj["tpg"]
    
    env_dir = helpers.create_environment_directories(TPG, env)
    
    # Change working directory to environment directory
    os.chdir(env_dir)

    # Find the selection CSV file(s) based on seed if provided
    if seed is not None:
        # Look for a specific seed file
        csv_files = glob.glob(os.path.join(env_dir, "logs", "selection", f"selection.{seed}.*.csv"))
        if not csv_files:
            raise click.ClickException(f"Could not find selection.{seed}.*.csv file. Ensure that you've evolved a policy with seed {seed}.")
    else:
        # No seed specified, find any selection file
        csv_files = glob.glob(os.path.join(env_dir, "logs", "selection", "selection.*.*.csv"))
        if not csv_files:
            raise click.ClickException("Ensure that you've evolved a policy before replaying it and the selection.*.*.csv file exists.")
    
    # Ensure misc directory exists for log files
    os.makedirs(os.path.join(env_dir, "logs", "misc"), exist_ok=True)
    
    executable = os.path.join(TPG, "build", "release", "experiments", "TPGExperimentMPI")
    
    # Process each CSV file (either one specific file or all files)
    for i, csv_file in enumerate(csv_files, 1):
        # Extract the seed from the filename (selection.SEED.*.csv)
        filename = os.path.basename(csv_file)
        file_parts = filename.split('.')
        file_seed = file_parts[1] if len(file_parts) > 2 else seed_aux
        
        # Get the best fitness, generation and team id
        metrics = helpers.get_metrics_from_csv(csv_file)
        
        # Get the checkpoint values
        checkpoint_in_phase, checkpoint_in_t = helpers.get_checkpoint_values(int(file_seed))
        
        # Build replay command
        cmd = [
            "mpirun", 
            "--oversubscribe",
            "-np", str(1),
            executable,
            f"parameters_file={hyper_parameters[env]}",
            f"seed_tpg={file_seed}",
            f"seed_aux={seed_aux}",
            f"start_from_checkpoint=1",
            f"checkpoint_in_phase={checkpoint_in_phase}",
            f"checkpoint_in_t={checkpoint_in_t}",
            f"replay=1", 
            f"animate=1",
            f"id_to_replay={int(metrics['team_id'])}",
            f"task_to_replay={task_to_replay}"
        ]
        
        stdout_file = f"logs/misc/tpg.{file_seed}.{seed_aux}.replay.std"
        stderr_file = f"logs/misc/tpg.{file_seed}.{seed_aux}.replay.err"
        
        click.echo(f"\nReplaying from file: {filename} ({i}/{len(csv_files)})")
        click.echo(f"Fitness: {metrics['best_fitness']}, Generation: {metrics['generation']}, Team ID: {metrics['team_id']}")
        
        click.echo(f"Launching MPI run with command:\n{' '.join(cmd)}")
        click.echo(f"Output will be written to {stdout_file} (stdout) and {stderr_file} (stderr).")
        
        try:
            with open(stdout_file, 'w') as stdout, open(stderr_file, 'w') as stderr:
                click.echo(f"Running replay for seed {file_seed}... (waiting for completion)")
                # Use subprocess.run() to wait for completion instead of Popen
                result = subprocess.run(cmd, stdout=stdout, stderr=stderr)
                if result.returncode != 0:
                    click.echo(f"Warning: Process for seed {file_seed} exited with code {result.returncode}")
                else:
                    click.echo(f"Replay for seed {file_seed} completed successfully")
        except OSError as e:
            raise click.ClickException(f"Failed to start experiment: {e}")

    total_replays = len(csv_files)
    click.echo(f"\nCompleted {total_replays} replay{'s' if total_replays > 1 else ''} sequentially")

@click.command(help="Cleanup an experiment directory")
@click.argument("env")
@click.pass_context
def clean(ctx: click.Context, env: str):
    """Remove the experiment directory"""

    # Fetch the hyperparameters for the environment
    hyper_parameters = ctx.obj["hyper_parameters"]
    TPG = ctx.obj["tpg"]
    
    env_dir = helpers.create_environment_directories(TPG, env)

    # Check if the environment directory exists
    if os.path.isdir(env_dir):
        # Remove the entire environment directory
        shutil.rmtree(env_dir)
        click.echo(f"Deleted environment directory: {env_dir}")
    else:
        click.echo(f"Environment directory not found: {env_dir}")

    click.echo("Cleanup completed.")

@click.command(help="Stop the evolution for the given environment")
@click.argument("env", required=False)
@click.pass_context
def kill(ctx: click.Context, env : str):
    """Terminate processes for the given environment"""
    try:
        if env is not None:
            hyper_parameters = ctx.obj["hyper_parameters"][env]
            result = subprocess.run(["pkill", "-f", hyper_parameters])
        else:
            result = subprocess.run(["pkill", "-f", "TPGExperimentMPI"])
        click.echo("Successfully killed processes.")
    except Exception as e:
        click.echo(f"Unexpected error: {e}")

@click.command(help="Debug a given environment")
@click.argument("env")
@click.option("-s", "--seed", help="Random seed", default=42)
@click.option("--seed-aux", help="Auxillary seed", default=42)
@click.pass_context
def debug(ctx: click.Context, env: str, seed : int, seed_aux : int):
    """Debug a given environment"""
    
    # Fetch the hyperparameters for the environment
    hyper_parameters = ctx.obj["hyper_parameters"]
    TPG = ctx.obj["tpg"]
    
    env_dir = helpers.create_environment_directories(TPG, env)
    
    # Change working directory to environment directory
    os.chdir(env_dir)
    
    executable = os.path.join(TPG, "build", "release", "experiments", "TPGExperimentMPI")
    
    # Build debugs command
    cmd = [
        "mpirun", 
        "--oversubscribe",
        "-np", str(2),
        "xterm",
        "-hold",
        "-e",
        "gdb",
        "-ex",
        "run",
        "--args",
        executable,
        f"parameters_file={hyper_parameters[env]}",
        f"seed_tpg={42}",
        f"n_root=10", 
        f"n_root_gen=10",
    ]
        
    stdout_file = f"logs/misc/tpg.{seed}.{seed_aux}.replay.std"
    stderr_file = f"logs/misc/tpg.{seed}.{seed_aux}.replay.err"
        
    click.echo(f"Launching MPI run with command:\n{' '.join(cmd)}")
    click.echo(f"Output will be written to {stdout_file} (stdout) and {stderr_file} (stderr).")
    
    try:
        with open(stdout_file, 'w') as stdout, open(stderr_file, 'w') as stderr:
            click.echo(f"Running debug for seed {seed_aux}...")
            # Use subprocess.run() to wait for completion instead of Popen
            result = subprocess.run(cmd, stdout=stdout, stderr=stderr)
            if result.returncode != 0:
                click.echo(f"Warning: Process for seed {seed_aux} exited with code {result.returncode}")
            else:
                click.echo(f"Debug for seed {seed_aux} completed successfully")
    except OSError as e:
        raise click.ClickException(f"Failed to start debug: {e}")
