import pandas as pd
import subprocess

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
