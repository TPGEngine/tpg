#!/usr/bin/env python3

import argparse
import pandas as pd
import matplotlib.pyplot as plt
import glob
import os
import sys
from matplotlib.cm import get_cmap

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
    
    tpg-plot-evolution.py all-selection best_fitness
    tpg-plot-evolution.py selection.42.42.csv,selection.42.43.csv best_fitness
    tpg-plot-evolution.py timing.42.42,timing.42.43 generation_time
    tpg-plot-evolution.py best_fitness
'''


def get_unique_filename(base_filename):
    """
    Generates a unique filename by appending an increment if the file already exists.
    
    :param base_filename: The base filename (e.g., 'output.png')
    :return: A unique filename (e.g., 'output_1.png', 'output_2.png', etc.)
    """
    filename, ext = os.path.splitext(base_filename)
    counter = 1
    new_filename = base_filename
    
    while os.path.exists(new_filename):
        new_filename = f"{filename}_{counter}{ext}"
        counter += 1
    
    return new_filename

def plot_generations(csv_files, column_name):
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
            df = pd.read_csv(csv_file)
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
    
    if not valid_files:
        print("No valid CSV files with required columns found!")
        return

    # configure properties of the graph (x-axis, y-axis, title, grid)
    plt.xlabel('Generation', fontsize=12)
    plt.ylabel(column_name, fontsize=12)
    title = f'{column_name} vs. Generations'
    
    if len(valid_files) > 1:
        title += f' ({len(valid_files)} files)'
    plt.title(title, fontsize=14)
    
    plt.grid(True, alpha=0.3)
    
    # if more than 1 file, add a legend of file names and color schemes
    if len(valid_files) > 1:
        plt.legend(bbox_to_anchor=(1.05, 1), loc='upper left')
    
    plt.tight_layout()

    output_filename = f"{column_name}_vs_generations"

    # multiple file plots will end with '_combined' 
    if len(valid_files) > 1:
        output_filename += "_combined"
    output_filename += ".png"

    # if output file already exists, add a number in the end
    output_filename = get_unique_filename(output_filename)
    
    plt.savefig(output_filename, bbox_inches='tight')
    print(f"Plot saved to '{output_filename}'")
    plt.close()

if __name__ == "__main__":
    # two arguments: CSV file(s) (optional) and column name (required)
    parser = argparse.ArgumentParser(description='Plot CSV column against generations')

    # csv_files defaults to 'all-selection' if left blank, plotting all CSV within the directory
    parser.add_argument('csv_files', type=str, nargs="?", default='all-selection', help='Comma-separated list of CSV files (or use "all" for *.csv)')
    parser.add_argument('column_name', type=str, help='Column name to plot against generations')
    
    args = parser.parse_args()
    
    # handle "all-*" keyword or comma-separated files
    if args.csv_files.lower() == 'all-selection':
        csv_files = glob.glob("selection.*.*.csv")
    elif args.csv_files.lower() == 'all-removal':
        csv_files = glob.glob("removal.*.*.csv")
    elif args.csv_files.lower() == 'all-timing':
        csv_files = glob.glob("timing.*.*.csv")
    elif args.csv_files.lower() == 'all-replacement':
        csv_files = glob.glob("replacement.*.*.csv")
    else:
        # add ".csv" in the end of filename if it doesn't already have it
        csv_files = [f.strip() + ".csv" if not f.strip().lower().endswith(".csv") else f.strip() 
                    for f in args.csv_files.split(',')]
        
        # verify all files have same prefix
        prefixes = {f.split('.')[0].lower() for f in csv_files}

        if len(prefixes) > 1 or not prefixes.issubset({"selection", "removal", "timing", "replacement"}):
            raise ValueError(f"All files must have the same prefix ('selection' or 'removal' or 'timing' or 'replacement'), but found: {prefixes}")

        # only plot unique CSV files
        csv_files = list(set(csv_files))

    valid_files = []
    for f in csv_files:
        if not os.path.exists(f):
            print(f"Warning: File '{f}' not found")
        else:
            valid_files.append(f)
    
    if not valid_files:
        print("No valid CSV files found!")
        sys.exit(1)
    
    try:
        plot_generations(valid_files, args.column_name)
    except Exception as e:
        print(f"Error: {str(e)}")
        sys.exit(1)
