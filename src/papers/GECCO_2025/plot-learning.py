# %%
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import os
dir = os.path.dirname(__file__)

# %%
def AddToPlot(reps, lab, col):
    reps_min = np.min(reps, 0)
    reps_max = np.max(reps, 0)
    reps_mean = np.mean(reps, 0)
    reps_std = np.std(reps, 0)
    plt.fill_between(range(len(reps[0])), reps_mean+0.5*reps_std, reps_mean-0.5*reps_std, alpha=0.5, color=col)
    plt.plot(range(len(reps[0])), reps_mean, label=lab, color=col)
    plt.plot(range(len(reps[0])), reps_max, label="", linestyle='dotted', color=col)
    plt.legend()

# %%


CB_color_cycle = ['#377eb8', '#ff7f00', '#4daf4a','#f781bf', '#a65628',
                  '#984ea3', '#999999', '#e41a1c', '#dede00']

path_1 = os.path.join(dir, 'data','TPGp')
path_2 = os.path.join(dir, 'data','TPG')

result_to_compare="training_rewards.csv"
max_generations=3500

df1 = pd.read_csv(path_1 + "/" + result_to_compare, sep='\s+', header=None, usecols=range(0,max_generations))
df2 = pd.read_csv(path_2 + "/" + result_to_compare, sep='\s+', header=None, usecols=range(0,max_generations))

fig = plt.figure(figsize=(6, 4))
AddToPlot(df1.to_numpy(),"TPGp", CB_color_cycle[0])
AddToPlot(df2.to_numpy(), "TPG", CB_color_cycle[1])
plt.xlabel('Generations')
plt.ylabel('Max Reward')
plt.show()