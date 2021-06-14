#! /usr/bin/env python

import argparse
import pandas as pd
import matplotlib.pyplot as plt


aparser = argparse.ArgumentParser()
aparser.add_argument('input', metavar='INPUT', type=str,
                     help='evaluation file')
args = aparser.parse_args()

data = pd.read_csv(args.input)
totals = data[['utilisation', 'cbs_tardiness/jobs', 'atlas_tardiness/jobs', 'total_tardiness/jobs']]
#grouped = totals.groupby("utilisation")
totals.boxplot(by='utilisation')
plt.show()
