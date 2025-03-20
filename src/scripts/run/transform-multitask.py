#!/usr/bin/env python3

'''
This script is used to retrieve individual data for each task within multi-task experiments.

It reads `.std` files within `logs/misc` and stores values to CSV files which are saved in `logs/selection` directory.

To use, follow these 2 steps:
1. `cd` to `experiments/<environment>`
2. Run `transform-multitask.py`.
'''

import os
import glob
import yaml
import csv
import sys

log_dir = "logs"
misc_dir = os.path.join(log_dir, "misc")
selection_dir = os.path.join(log_dir, "selection")

std_file_pattern = "tpg.*.*.std"
column_list = ["generation", "best_fitness", "effective_program_instruction_count"]

def get_project_root(filename=".devcontainer"):
    path = os.getcwd()
    while path != os.path.dirname(path):
        if os.path.exists(os.path.join(path, filename)):
            return path
        path = os.path.dirname(path)
    
    raise ValueError("Error: Cannot find project root.")

def get_config_yaml_file(target_name):
    project_root = get_project_root()
    config_dir = os.path.join(project_root, "configs")

    for file in os.listdir(config_dir):
        target_file = "mujoco_" + target_name + ".yaml"
        if file.lower() == target_file:
            yaml_file_path = os.path.join(config_dir, file)
            
            print(f"Found YAML file: {yaml_file_path}")
            return yaml_file_path
    
    raise ValueError("Error: No config YAML file found.")

def get_active_tasks_from_configs():
    current_dir_name = os.path.basename(os.getcwd())
    config_file_path = get_config_yaml_file(current_dir_name)

    if os.path.exists(config_file_path):
        with open(config_file_path, "r") as file:
            data = yaml.safe_load(file)
            active_tasks = data["general_task_parameters"]["active_tasks"].split(",")

            if len(active_tasks) > 1:
                # Removing 'Mujoco_' and '_v4' from each element
                active_tasks = [item.replace('Mujoco_', '').replace('_v4', '') for item in active_tasks]
                return active_tasks
            else:
                raise ValueError("Error: Only one or no active task found in the configuration.")

    raise ValueError("Error: Config file does not exist.")

def get_seed_and_pid(filename):
    parts = filename.split('.')
    _, seed, pid, _ = parts

    return [seed, pid]

def process_std_files():
    std_files = glob.glob(os.path.join(misc_dir, std_file_pattern))
    active_tasks = get_active_tasks_from_configs()

    for file_path in std_files:
        filename = os.path.basename(file_path)
        seed, pid = get_seed_and_pid(filename)

        print(f"Processing: {filename}")

        csv_data = {task: [column_list] for task in active_tasks}
        with open(file_path, "r") as file:
            generation = 0
            for line in file:
                words = line.strip()
                if not words.startswith("setElTmsMTA"): continue

                words = words.split()
                if not words: continue

                key_value_pairs = words[1:]
                current_task, key_idx = 0, 0
                line_dict = {task: [None] * (len(column_list) - 1) for task in active_tasks}

                for i in range(0, len(key_value_pairs)):
                    key = key_value_pairs[i]
                    keys_to_store = [f"p0t{current_task}a0", f"p0t{current_task}a2"]

                    if key in keys_to_store and i + 1 <= len(key_value_pairs):
                        value = key_value_pairs[i + 1]
                        line_dict[active_tasks[current_task]][key_idx] = value
                    
                        key_idx += 1

                        if key == keys_to_store[-1]:
                            current_task += 1
                            key_idx = 0

                for task, arr_data in line_dict.items():
                    csv_data[task].append(
                            [generation] + arr_data
                        )

                generation += 1

        for task, data in csv_data.items():
            output_filename = f"selection.{seed}.0_{task}.csv"
            output_path = os.path.join(selection_dir, output_filename)

            with open(output_path, "w", newline="") as csv_file:
                writer = csv.writer(csv_file)
                writer.writerows(data)

            print(f"Saved: {output_path}")

if __name__ == "__main__":
    process_std_files()
