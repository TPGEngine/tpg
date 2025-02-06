import pandas as pd
import matplotlib.pyplot as plt
import pathlib
import sys

file = sys.argv[1]
p = pathlib.Path(file)

plt.rcParams["figure.figsize"] = [7.50, 3.50]
plt.rcParams["figure.autolayout"] = True

df = pd.read_csv(p)

# df.plot(plt.plot(df.Name, df.Marks))
df.plot(x='Time', y=['x0', 'y0'])

# draw prime lines
# plt.ylim(0.3,0.85)
plt.hlines(y=0.93, xmin=950, xmax=999, linewidth=2, color='r')
plt.vlines(x=950, ymin=0.91, ymax=0.93, linewidth=2, color='r')
plt.vlines(x=999, ymin=0.91, ymax=0.93, linewidth=2, color='r')
plt.text(975, 0.97, 'prime steps', ha='center', va='center')

# plt.xticks(x, labels, rotation=45)
# plt.legend()#['Target', 'Prediction'])
plt.legend(['Target', 'Prediction'], ncol=2, frameon=True, loc='upper right', bbox_to_anchor=(1.005, 1.15))

plt.ylim(-.05,1.05)
# plt.ylim(0,128)
plt.savefig(p.with_name(p.name.split('.')[0]).with_suffix('.pdf'))
plt.show()