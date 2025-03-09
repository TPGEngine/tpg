import pandas as pd
import subprocess
import os
import click

def get_metrics_from_csv(csv_file):
    # Read the CSV file
    df = pd.read_csv(csv_file)

    # Find the maximum best_fitness value
    max_fitness = df["best_fitness"].max()

    # Filter the rows with that best_fitness
    filtered = df[df["best_fitness"] == max_fitness]

    # From those rows, select the one with the lowest generation
    result_row = filtered.loc[filtered["generation"].idxmin()]

    # Extract only the columns of interest
    return result_row[["best_fitness", "generation", "team_id"]].to_dict()   

def get_checkpoint_values(seed_tpg):
    # First command to get checkpoint_in_phase
    cmd_phase = (
        f"grep -iRl end checkpoints/cp.*.{seed_tpg}.*.rslt | "
        "cut -d '.' -f 4 | sort -n | tail -n 1"
    )
    # Run the command and decode its output
    checkpoint_in_phase = subprocess.check_output(cmd_phase, shell=True,
                                                text=True).strip()

    # Second command to get checkpoint_in_t using the obtained checkpoint_in_phase
    cmd_t = (
        f"grep -iRl end checkpoints/cp.*.{seed_tpg}.{checkpoint_in_phase}.rslt | "
        "cut -d '.' -f 2 | sort -n | tail -n 1"
    )
    checkpoint_in_t = subprocess.check_output(cmd_t, shell=True,
                                            text=True).strip()
    
    return checkpoint_in_phase, checkpoint_in_t

def create_environment_directories(TPG: str, env: str):
    """Create directory structure for environment experiments"""
    
    # Convert environment name to snake_case for directory naming
    env_dir = os.path.join(TPG, "experiments", env)
    
    directories = [
        os.path.join(env_dir, "checkpoints"),
        os.path.join(env_dir, "frames"),
        os.path.join(env_dir, "graphs"),
        os.path.join(env_dir, "logs", "misc"),
    ]
    
    for directory in directories:
        os.makedirs(directory, exist_ok=True)
        # Create .gitignore in each directory
        gitignore_path = os.path.join(directory, ".gitignore")
        if not os.path.exists(gitignore_path):
            with open(gitignore_path, "w") as f:
                f.write("*\n!.gitignore\n")
                
    return env_dir

def run_single_experiment(executable: str, parameters_file: str, processes: int, seed: int):
    """Run a single TPG experiment with the specified parameters"""
    
    pid = os.getpid()
    stdout_file = f"logs/misc/tpg.{seed}.{pid}.std"
    stderr_file = f"logs/misc/tpg.{seed}.{pid}.err"
    
    cmd = [
        "mpirun", 
        "--oversubscribe",
        "-np", str(processes),
        executable,
        f"parameters_file={parameters_file}",
        f"seed_tpg={seed}",
        f"pid={pid}"
    ]

    click.echo(f"Launching MPI run with command:\n{' '.join(cmd)}")
    click.echo(f"Output will be written to {stdout_file} (stdout) and {stderr_file} (stderr).")

    try:
        with open(stdout_file, 'w') as stdout, open(stderr_file, 'w') as stderr:
            process = subprocess.Popen(cmd, stdout=stdout, stderr=stderr)
            click.echo(f"Process started in the background with PID: {process.pid}")
    except OSError as e:
        raise click.ClickException(f"Failed to start experiment: {e}")

    click.echo("Evolve (started in background)")

