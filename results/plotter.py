#!/usr/bin/env python
# Author: Mohammed Sourouri <mohammed.sourouri@ntnu.no>
# Date: January 16, 2017
# Updated: September 12, 2017

import sys
import argparse
import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt

from utils import delete_ds_store, canonize, exists
from sys_cfg import static_sys_cfgs, best_static_cfgs
from matplotlib import rcParams

# Set style
plt.style.use('classic')
rcParams['pdf.fonttype'] = 42
rcParams['ps.fonttype'] = 42


def barplot(objective, sys_cfgs, pcfg):

    ax = plt.figure(figsize=(10, 6)).add_subplot(111)

    data = []
    for cfg in sys_cfgs:
        cfg_dict = best_static_cfgs(objective, cfg)
        data.append(cfg_dict[objective])

    df = pd.DataFrame.from_records(data)
    df['Size'] = pd.to_numeric(df['Size'])

    sns.barplot(x=df['Size'].values, y=canonize(objective), ax=ax, hue='Implementation', data=df,
                ci=None, palette='muted')

    # Style the plot
    ax.set_title(pcfg['title'], fontsize=12, fontweight='bold')
    ax.grid(False)
    ax.set_facecolor('w')
    ax.legend(loc='upper center', ncol=len(df['Implementation'].unique()), fancybox=True, shadow=True)
    ax.set_xticklabels(df['Size'].unique(), horizontalalignment='right', rotation=40)
    ax.set_xlabel(pcfg['xlabel'], fontweight='bold')
    ax.set_ylabel(pcfg['ylabel'], fontweight='bold')
    ax.set_ylim(0, 140)

    plt.savefig('figures/optewe'+pcfg['filename']+'.pdf', format='PDF', bbox_inches='tight', dpi=1200)
#    plt.show()


def lineplot(objective, sys_cfgs, pcfg):

    ax = plt.figure().add_subplot(111)

    for df in sys_cfgs:
        size = pd.to_numeric(df['Size'])
        x = pd.to_numeric(df['Threads'])
        y = pd.to_numeric(df[canonize(objective)])
        ax.plot(x.sort_values(), y.sort_values(), label=r'${}^3$'.format(size.unique()[0]), linewidth=1.5)
        ax.xaxis.set_ticks(x)
        ax.set_xticklabels(x, color='black')

    # Style the plot
    ax.set_title(pcfg['title'], fontsize=12, fontweight='bold')
    ax.grid(False)
    ax.set_facecolor('w')
    ax.legend(loc='upper center', ncol=len(sys_cfgs), fancybox=True, shadow=True)
    ax.set_xlabel(pcfg['xlabel'], fontweight='bold')
    ax.set_ylabel(pcfg['ylabel'], fontweight='bold')
    ax.set_ylim(0, 140)

    filename_postfix = ''
    if df['Implementation'].unique() == 'AVX2':
        filename_postfix = '_avx'

    plt.savefig('figures/optewe'+pcfg['filename']+filename_postfix+'.pdf', format='PDF', bbox_inches='tight', dpi=1200)
#    plt.show()


def plot_cfg(objective, objectives, plot_type):

    title_dict = dict.fromkeys(objectives, 'OptEWE: Performance Comparision')
    ylabel_dict = {'mlups': 'Mega Lattice Update Per Second [MLUPs]', 'runtime': 'Milliseconds (ms)',
                   'edp': 'Energy Delay Product [EDP]', 'ed2p': r'Energy Delay Product Squared [$ED^{2}P$]'}

    xlabel_dict = {'line': 'Number of Threads', 'bar': 'Model Size'}

    cfg = {'title': title_dict[objective], 'ylabel': ylabel_dict[objective], 'xlabel': xlabel_dict[plot_type],
           'ylim': 140, 'filename': '_'+objective+'_'+plot_type}

    return cfg


def main():

    parser = argparse.ArgumentParser(description='A Simple Plot Generator')
    parser.add_argument('-t', '--type', help='Type of plot. Valid types are: [line | bar]', required=True)
    parser.add_argument('-o', '--objective', help='Tuning function objective.'
                                                  'Valid objectives are: [runtime | energy | edp | ed2p | mlups]',
                        required=True)
    parser.add_argument('-d', '--directories', nargs='*', help='Result directories', required=True)
    args = vars(parser.parse_args())

    objectives = {'runtime', 'energy', 'edp', 'ed2p', 'mlups'}
    plot_types = {'line': lineplot, 'bar': barplot}

    objective_exist = exists(args.values(), objectives)
    plot_type_exist = exists(args.values(), plot_types)

    if not objective_exist:
        print 'Valid objectives are: [runtime | energy | edp | ed2p | mlups]'
        sys.exit(2)

    if not plot_type_exist:
        print 'Valid plot types are: [line | bar]'
        sys.exit(2)

    # Get the sys cfg for the input directories
    sys_cfgs = []
    for directory in args['directories']:
        delete_ds_store(directory)
        sys_cfgs.append(static_sys_cfgs(directory))

    # Plot cfg
    pcfg = plot_cfg(args['objective'].lower(), objectives, args['type'])

    # Plot
    plot_types[args['type']](args['objective'].lower(), sys_cfgs, pcfg)

if __name__ == '__main__':
    main()
