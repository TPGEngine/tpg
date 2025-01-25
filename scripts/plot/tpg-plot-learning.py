# %%
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

# %%
def AddToPlot(reps):
    reps_mean = np.mean(reps, 0)
    reps_std = np.std(reps, 0)
    plt.fill_between(range(len(reps[0])), reps_mean+0.5*reps_std, 
      reps_mean-0.5*reps_std, alpha=0.5)
    plt.plot(range(len(reps[0])), reps_mean)

# %%

path_1="/home/skelly/experiments/mujoco-constants-2024-12-06/mujoco-Reacher-2024-12-05-22-21-08-263a646"
path_2="/home/skelly/experiments/mujoco-constants-2024-12-06/mujoco-Reacher-2024-12-05-22-21-09-263a646"

result_to_compare="aux_0_ST_0_p0.csv"

df1 = pd.read_csv(path_1 + "/" + result_to_compare, sep='\s+', header=None, on_bad_lines='warn')
df2 = pd.read_csv(path_2 + "/" + result_to_compare, sep='\s+', header=None, on_bad_lines='warn')

fig = plt.figure(figsize=(6, 4))
AddToPlot(df1.to_numpy())
AddToPlot(df2.to_numpy())
# df1.T.plot()
# df2.T.plot()
plt.show()