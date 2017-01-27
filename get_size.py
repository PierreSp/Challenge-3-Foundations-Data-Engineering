import pandas as pd
dt = pd.read_csv('playedin.csv')
dt.set_index('movie', inplace=True)
joined = dt.join(dt, lsuffix='_l')
print(len(joined))
