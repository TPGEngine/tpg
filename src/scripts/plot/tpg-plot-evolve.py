#!/usr/bin/env python3

import argparse
import pandas as pd
import matplotlib.pyplot as plt
import glob
import os
import sys
import csv
import re
import numpy as np
from matplotlib.cm import get_cmap
from matplotlib.backends.backend_pdf import PdfPages

'''
The script is included within the environmental variables for TPG.

A comprehensive guide can be found in the link below.

https://gitlab.cas.mcmaster.ca/kellys32/tpg/-/wikis/TPG-Generation-Plot-for-CSV-Logging-Files.

To use, simply call the script inside any experiments within `experiment_directories/*`.
    
    :param csv_files (Optional): The name of csv file(s) to plot
    :param column_name (Required): The name of the column to plot against generations 
    :return: Returns nothing but saves a .png file for the plot

For the optional parameter `csv_files`, possible values can be:
    
    - 'all-selection' | 'all-removal' | 'all-timing' | 'all-replacement' (retrieves all CSV files within the directory)
    - 'csv_file1.csv,csv_file2.csv, ...' (can have one or more specific CSV files listed with same type, separated by comma `,`)
    - 'csv_file1,csv_file2, ...' (similar to above, but doesn't need the ".csv" extension in the file name)
    - left blank (the program is setup to default to 'all-selection')

Example:
    
    tpg-plot-evolve.py all-selection best_fitness
    tpg-plot-evolve.py selection.42.42.csv,selection.42.43.csv best_fitness
    tpg-plot-evolve.py timing.42.42,timing.42.43 generation_time
    tpg-plot-evolve.py best_fitness
'''

log_dir = "logs"
plot_dir = "plots"

def get_unique_filename(base_filename):
    """
    Generates a unique filename by appending an increment if the file already exists.
    
    :param base_filename: The base filename (e.g., 'output.png')
    :return: A unique filename (e.g., 'output_1.png', 'output_2.png', etc.)
    """
    filename, ext = os.path.splitext(base_filename)
    counter = 1
    new_filename = base_filename
    
    while os.path.exists(f"{plot_dir}/{new_filename}"):
        new_filename = f"{filename}_{counter}{ext}"
        counter += 1
    
    return new_filename

def get_csv_columns(filepath):
    """Extracts column names from a CSV file."""
    with open(filepath, newline="", encoding="utf-8") as file:
        reader = csv.reader(file)
        column_names = next(reader)
        return column_names[1:]

def capitalize_snake_case(s):
    return ' '.join(word.capitalize() for word in s.split('_'))

def plot_generations_single(csv_files, column_name, pdf = None, num_x = None, num_y = None):
    """
    Plots the given csv_files and column name against generations.
    
    :param csv_files: The name of csv file(s) to plot (e.g., 'tma.42.12345.csv')
    :param column_name: The column to plot against 'generation' (e.g., 'best_fitness')
    :return: None, but saves the output into a .png file
    """
    plt.figure(figsize=(12, 7))
    cmap = plt.get_cmap('tab10') # a list of 10 color schemes
    line_styles = ['-', '--', '-.', ':']
    
    valid_files = []
    
    # start processing the listed csv files
    for idx, csv_file in enumerate(csv_files):
        try:
            # Use full path to read the CSV file
            full_path = os.path.join(log_dir, csv_file)
            df = pd.read_csv(full_path)
            
            if 'generation' not in df.columns:
                print(f"Skipping {csv_file}: missing 'generation' column")
                continue
            
            if column_name not in df.columns:
                print(f"Skipping {csv_file}: missing '{column_name}' column")
                continue
            
            # configure line style and color scheme of current .csv file values
            color = cmap(idx % 10) # line color of the current file is decided using % 10
            line_style = line_styles[(idx // 10) % len(line_styles)] # with 10 color schemes, the line style is changed after 10 iterations
            label = os.path.splitext(os.path.basename(csv_file))[0]

            if num_x is not None:
                df = df.head(num_x)  # keep only the first `num_x` rows from dataframe
            
            plt.plot(df['generation'], df[column_name], 
                    marker='o' if len(csv_files) == 1 else '',
                    linestyle=line_style,
                    color=color,
                    label=label,
                    alpha=0.8)
            
            # necessary to keep track whether files are being processed
            valid_files.append(csv_file)
            
        except Exception as e:
            print(f"Error processing {csv_file}: {str(e)}")

    if num_y is not None:
        plt.ylim(0, num_y)  # set y-axis range from 0 to `num_y`

    if not valid_files:
        print("No valid CSV files with required columns found!")
        return

    # configure properties of the graph (x-axis, y-axis, grid)
    plt.xlabel('Generation', fontsize=14)
    plt.ylabel(capitalize_snake_case(column_name), fontsize=14)

    plt.grid(True, alpha=0.3)
    plt.tick_params(axis='both', which='major', labelsize=16)

    # only show integers for `generation` x-axis
    plt.gca().xaxis.set_major_locator(plt.MaxNLocator(integer=True))
    
    # if more than 1 file, add a legend of file names and color schemes
    if len(valid_files) > 1:
        plt.legend(bbox_to_anchor=(1.05, 1), loc='upper left')
    
    plt.tight_layout()
    
    if pdf:
        pdf.savefig()
    else:
        output_filename = f"{column_name}_vs_generations"

        # multiple file plots will end with '_combined' 
        if len(valid_files) > 1:
            output_filename += "_combined"
        output_filename += ".pdf"

        # if output file already exists, add a number in the end
        output_filename =  f"{plot_dir}/{get_unique_filename(output_filename)}"

        plt.savefig(output_filename)
        print(f"Plot saved to '{output_filename}'")

    plt.close()

def plot_generations_multiple(csv_files, column_names, num_x, num_y):
    # Get prefix from the first file - use basename to handle subdirectory structure
    first_file = os.path.basename(csv_files[0]) if csv_files else ""
    prefix = first_file.split('.')[0] if first_file else ""

    if not os.path.exists(plot_dir):
        os.makedirs(plot_dir)

    output_filename = plot_dir + "/" + get_unique_filename(f"{prefix}_all_vs_generations.pdf")

    with PdfPages(output_filename) as pdf:
        for column_name in column_names:
            plot_generations_single(csv_files, column_name, pdf, num_x, num_y)

    print(f"All plots saved to '{output_filename}'")

if __name__ == "__main__":
    # two arguments: CSV file(s) (optional) and column name (required)
    parser = argparse.ArgumentParser(description='Plot CSV column against generations')

    # csv_files defaults to 'all-selection' if left blank, plotting all CSV within the directory
    parser.add_argument('csv_files', type=str, nargs="?", default='all-selection', help='Comma-separated list of CSV files (or use "all" for *.csv)')
    parser.add_argument('column_name', type=str, help='Column name to plot against generations')

    # new arguments for number of x and y-axes
    parser.add_argument('--num-x', type=int, help='Number of x-axes')
    parser.add_argument('--num-y', type=int, help='Number of y-axes')
    
    args = parser.parse_args()
    
    # handle "all-*" keyword or comma-separated files
    prefixes = {"selection", "removal", "timing", "replacement"}
    csv_key = args.csv_files.lower()

    if csv_key.startswith("all-"):
        prefix = csv_key[4:]  # extract part after "all-"
        if prefix in prefixes:
            # Use direct path construction - don't rely on chdir
            prefix_dir = os.path.join(log_dir, prefix)
            
            if not os.path.exists(prefix_dir):
                raise ValueError(f"Subdirectory '{prefix}' not found in {log_dir}")
            
            csv_files = [
                f for f in os.listdir(prefix_dir) 
                if re.match(rf"^{prefix}\.\d+\.\d+\.csv$", f)
            ]

            if not csv_files:
                print(f"No CSV files found in {prefix_dir}")
                sys.exit(1)
                
            # Store paths relative to log_dir
            csv_files = [os.path.join(prefix, f) for f in csv_files]
        else:
            raise ValueError(f"Invalid prefix '{prefix}'. Expected one of {prefixes}.")
    else:
        # Extract files and determine their prefixes
        raw_files = [f.strip() for f in args.csv_files.split(',')]
        csv_files = []
        
        for f in raw_files:
            if not f.lower().endswith(".csv"):
                f = f"{f}.csv"
                
            # Extract prefix from filename
            prefix = f.split('.')[0].lower()
            if prefix not in prefixes:
                raise ValueError(f"File '{f}' doesn't have a valid prefix. Expected one of {prefixes}.")
            
            # Store with subdirectory
            csv_files.append(os.path.join(prefix, f))

        csv_files = list(set(csv_files))  # remove duplicates
    
    valid_files = []
    for f in csv_files:
        full_path = os.path.join(log_dir, f)
        if not os.path.exists(full_path):
            print(f"Warning: File '{full_path}' not found")
        else:
            valid_files.append(f)
    
    if not valid_files:
        print("No valid CSV files found!")
        sys.exit(1)

    column_name = args.column_name
    num_x = args.num_x
    num_y = args.num_y

    try:
        if column_name == "all":
            # Get columns from the first valid file using full path
            full_path = os.path.join(log_dir, valid_files[0])
            column_names = get_csv_columns(full_path)
            plot_generations_multiple(valid_files, column_names, num_x, num_y)
        else:
            plot_generations_single(valid_files, column_name, num_x = num_x, num_y = num_y)
    except Exception as e:
        print(f"Error: {str(e)}")
        sys.exit(1)
