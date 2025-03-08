import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import pathlib
import sys
from matplotlib.lines import Line2D 

truncate_gen=2000

def clip_generations(df):
  # Find the index of the first row with any NaN values
  first_nan_index = df[df.isna().any(axis=1)].index.min()

  if pd.isna(first_nan_index):  # Check if no NaN values were found
    truncated_df = df
  else:
    # Truncate the DataFrame
    truncated_df = df.loc[:first_nan_index-1]
  return truncated_df

def plot_experiment(arg, col, lab):
  file = sys.argv[arg]
  p = pathlib.Path(file)
  df = pd.read_csv(p, header=None, sep="\s+",names=range(max_gen))
  df = df.T
  # df = clip_generations(df)
  #truncate to generation 2000
  df = df.loc[:truncate_gen]
  medians = df.median(axis='columns')
  # Plot all lines.
  # plt.plot(df, color=col, alpha=alph)
  plt.plot(medians.index, medians.values, linestyle='-', color=col, label=lab)

  x = np.linspace(0,len(df.axes[0]),len(df.axes[0]))
  mins = df.min(axis='columns')
  maxs = df.max(axis='columns')
  plt.fill_between(x, maxs, mins, color=col, alpha=0.4)
  

max_gen = 15000
plot_name = sys.argv[1]
# plt.rcParams["figure.figsize"] = [7.50, 3.50]
plt.rcParams["figure.figsize"] = [6, 4]
plt.rcParams["figure.autolayout"] = True

cmap = plt.get_cmap("tab10")

plot_experiment(2,cmap(0), "scalar,vector,matrix")
plot_experiment(3,cmap(1), 'scalar')

custom_lines = [Line2D([0], [0], color=cmap(0), lw=4),
                Line2D([0], [0], color=cmap(1), lw=4)]
plt.legend(title='Operation Types',loc='upper left')
plt.xlabel("Generation")
plt.ylabel("Instructions per Prediction")

# plt.show()
plt.savefig(plot_name)