#! /usr/bin/env python

import argparse
import pandas as pd
import matplotlib.pyplot as plt


aparser = argparse.ArgumentParser()
aparser.add_argument('input', metavar='INPUT', type=str,
                     help='evaluation file')
args = aparser.parse_args()

data = pd.read_csv(args.input)
#grouped = totals.groupby("utilisation")
data.boxplot(column='cbs_counter/jobs', by='utilisation')
data.boxplot(column='atlas_counter/jobs', by='utilisation')
data.boxplot(column='total_counter/jobs', by='utilisation')
plt.show()
