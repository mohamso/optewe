#!/usr/bin/env python
# Author: Mohammed Sourouri <mohammed.sourouri@ntnu.no>
# Date: February 20, 2017
# Update: March 21, 2017

import argparse
import matplotlib.pyplot as plt
import seaborn as sns
from sys_cfg import dynamic_sys_cfgs


def plot_ktimes(args, data):

    # Set style
    plt.style.use('classic')

    # Set up figure
    ax = plt.figure().add_subplot(111)

    g = sns.barplot(x=data.transpose()['Kernel Name'], y=data.transpose()['Runtime'], ax=ax,
                    palette=sns.color_palette('muted', len(data.transpose()['Kernel Name'])))
    g.set_xticklabels(data.transpose()['Kernel Name'], fontweight='bold', fontsize=10, rotation=90)

    ax.set_facecolor('w')
    ax.set_ylabel(r'Time (milliseconds)', fontweight='bold', fontsize=12)
    ax.set_title(r'OptEWE: Kernel Runtime Comparison (${}^3$)'.format(args['size'])
                 + ', Threads='+str(args['threads'])+'', fontweight='bold', fontsize=12)

    # Uncomment to draw a red line at 100ms
    #ax.axhline(y=100, xmin=0, xmax=3, c='red', linewidth=1.0, zorder=2)

    plt.tight_layout(h_pad=3)

    # Save and show figure
    plt.savefig('figures/optewe_'+args['size']+'_ktimes_t'+str(args['threads'])+'.pdf',
                format='PDF', bbox_inches='tight', dpi=1200)
#    plt.show()


def ktimes(args):

    df = dynamic_sys_cfgs(args['directory'])
    df_t = df.transpose()
    return df_t.loc[:, df_t.loc['Threads'] == int(args['threads'])]


def main():

    parser = argparse.ArgumentParser(description='Kernel Times Plotter')
    parser.add_argument('-s', '--size', help='Problem size.', required=True)
    parser.add_argument('-t', '--threads', help='Number of threads.', required=True)
    parser.add_argument('-d', '--directory', help='Result directory', required=True)
    args = vars(parser.parse_args())

    # Get kernel times
    data = ktimes(args)

    # Plot data
    plot_ktimes(args, data)


if __name__ == '__main__':
    main()
