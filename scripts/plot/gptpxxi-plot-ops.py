import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import pathlib
import sys

plt.rcParams["figure.figsize"] = [7.50, 3.50]
plt.rcParams["figure.autolayout"] = True
plot_name = sys.argv[1]
file = sys.argv[2]
p = pathlib.Path(file)
df = pd.read_csv(p, header=None)

bp = plt.boxplot(df)

plt.savefig(plot_name)
# plt.show()