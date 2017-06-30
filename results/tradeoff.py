#!/usr/bin/env python
# Author: Mohammed Sourouri <mohammed.sourouri@ntnu.no>
# Date: March 24, 2017

import sys
import argparse
import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt

from sys_cfg import static_sys_cfgs, best_static_cfg, dynamic_sys_cfgs, best_dynamic_sys_cfg, ideal_dyn_cfg
from utils import delete_ds_store, exists, canonize


def barplot(objective, scale, data, avx_data, pcfg):

    # Set style
    plt.style.use('classic')
    f, ax = plt.subplots(figsize=(5, 5))
    labels = ["code", "reference", "static", "dynamic"]

    no_avx = data[canonize(objective)];
    avx = avx_data[canonize(objective)];
    assert(len(no_avx) == 3 and len(avx) == 3)
    avx_values = ("AVX2", avx.values[0], avx.values[1], avx.values[2])
    noavx_values = ("No-AVX2", no_avx.values[0], no_avx.values[1], no_avx.values[2]);

    frame = pd.DataFrame.from_records([noavx_values, avx_values], columns=labels)
    print frame

    sns.set_color_codes("colorblind")
    sns.barplot(data=frame, x="code", y="reference", label="Reference", color="b")
    sns.barplot(data=frame, x="code", y="static", label="Static", color="r")
    sns.barplot(data=frame, x="code", y="dynamic", label="Dynamic", color="g")

    # Style the plot
    ax.set_title(str(pcfg['title']).upper(), fontsize=12, fontweight='bold', y=1.04)
    ax.grid(False)
    ax.set_facecolor('w')
    ax.legend(loc='lower right', fancybox=True, shadow=True)
    ax.set_xlabel(pcfg['xlabel'] + "=" + str(pd.to_numeric(data['Size'].values[0])), fontweight='bold')
    ax.set_ylabel(pcfg['ylabel'], fontweight='bold')
    ax.set_ylim(float(scale[0]), float(scale[1]))

    plt.tight_layout(h_pad=1)

    plt.savefig('figures/optewe'+pcfg['filename']+str(data['Size'].values[0])+'.pdf', format='PDF',
                bbox_inches='tight', dpi=1200)
#    plt.show()


def plot_cfg(objective, objectives):

    title_dict = dict.fromkeys(objectives, 'OptEWE: Tuning Objective - '+objective.title())
    ylabel_dict = {'mlups': 'Mega Lattice Update Per Second [MLUPs]', 'runtime': 'Seconds (sec)',
                   'edp': 'Energy Delay Product [EDP]', 'ed2p': r'Energy Delay Product Squared [$ED^{2}P$]',
                   'energy': 'Joules'}

    cfg = {'title': title_dict[objective], 'ylabel': ylabel_dict[objective], 'xlabel': 'Model Size',
           'filename': '_dynamic_'+objective.lower()}

    return cfg


def tradeoff(objective, ref_cfg, static_cfg, ideal_cfg):

    ref_ctime = round(ref_cfg[canonize('runtime')].iloc[0], 2)

    static_ctime = round(static_cfg[canonize('runtime')].iloc[0], 2)
    dyn_ctime = round(ideal_cfg[canonize('runtime')], 2)

    print 'The objective is to min(', objective, ')'
    print 'The runtime of the reference cfg:', ref_ctime, 'seconds'
    print 'The runtime of the best found static cfg:', static_ctime, 'seconds'
    print 'The runtime of the best found dynamic cfg:', dyn_ctime, 'seconds'

    # Performance trade-offs
    stat_perf_trade_off = round((static_ctime-ref_ctime)/ref_ctime * 100.0, 2)
    print 'Compute time: ' + str(stat_perf_trade_off) + ' % static compared to reference implementation'

    ref_perf_trade_off = round((dyn_ctime-ref_ctime)/ref_ctime * 100.0, 2)
    print 'Compute time: ' + str(ref_perf_trade_off) + ' % dynamic compared to reference implementation'

    perf_trade_off = round((dyn_ctime-static_ctime)/static_ctime * 100.0, 2)
    print 'Compute time: ' + str(perf_trade_off) + ' % dynamic compared to static tuning'

    # Energy saved
    ref_energy = round(ref_cfg[canonize('energy')].iloc[0], 2)
    stat_energy = round(static_cfg[canonize('energy')].iloc[0], 2)
    dyn_energy = round(ideal_cfg[canonize('energy')], 2)

    static_energy_savings = round(((stat_energy -ref_energy) / ref_energy) * 100.0, 2)
    dyn_ref_energy_savings = round(((dyn_energy - ref_energy) / ref_energy) * 100.0, 2)
    dyn_stat_energy_savings = round(((dyn_energy -stat_energy) / stat_energy) * 100.0, 2)

    print 'The energy consumption of the reference cfg:', ref_energy
    print 'The energy consumption of the static cfg:', stat_energy
    print 'The energy consumption of the dynamic cfg:', dyn_energy

    print 'Energy saved: ' + str(static_energy_savings) + ' % static compared to reference implementation'
    print 'Energy saved: ' + str(dyn_ref_energy_savings) + ' % dynamic compared to reference tuning'
    print 'Energy saved: ' + str(dyn_stat_energy_savings) + ' % dynamic compared to static tuning'


def main():

    parser = argparse.ArgumentParser(description='A tool that computes the energy-performance tradeoff')
    parser.add_argument('-o', '--objective', help='Tuning objective.', required=True)
    parser.add_argument('-s', '--scale', nargs=2, help='Scale.', required=True)
    parser.add_argument('-d', '--directories', nargs='*', help='Result directories', required=True)
    args = vars(parser.parse_args())

    objectives = {'runtime', 'energy', 'edp', 'ed2p'}

    objective_exist = exists(args.values(), objectives)

    if not objective_exist:
        print 'Valid objectives are: [runtime | energy | edp | ed2p]'
        sys.exit(2)

    # Get the reference, static and dynamic sys cfg for the input directories
    for directory in args['directories']:
        delete_ds_store(directory)

        cfg = best_static_cfg(canonize('runtime'), static_sys_cfgs(directory))

        if cfg['Implementation'].values == 'Reference':
            ref_sys_cfg = cfg
        elif cfg['Implementation'].values == 'Reference (ST)':
            static_sys_cfg = best_static_cfg(args['objective'], static_sys_cfgs(directory))
            dyn_cfg = best_dynamic_sys_cfg(args['objective'], dynamic_sys_cfgs(directory))
            ideal_cfg = ideal_dyn_cfg(dyn_cfg)
        elif cfg['Implementation'].values == 'AVX2':
            ref_avx_sys_cfg = cfg
        elif cfg['Implementation'].values == 'AVX2 (ST)':
            static_avx_sys_cfg = best_static_cfg(args['objective'], static_sys_cfgs(directory))
            dyn_avx_cfg = best_dynamic_sys_cfg(args['objective'], dynamic_sys_cfgs(directory))
            ideal_avx_cfg = ideal_dyn_cfg(dyn_avx_cfg)

    # Create a dataframe for dynamic
    ideal_cfg['Size'] = pd.to_numeric(ref_sys_cfg['Size']).values[0]
    ideal_cfg['Iteration'] = pd.to_numeric(ref_sys_cfg['Iteration']).values[0]
    ideal_cfg['Implementation'] = 'Dynamic'
    ideal_cfg_df = pd.DataFrame(ideal_cfg, index=[0])

    # Create a dataframe for dynamic avx
    ideal_avx_cfg['Size'] = pd.to_numeric(ref_sys_cfg['Size']).values[0]
    ideal_avx_cfg['Iteration'] = pd.to_numeric(ref_sys_cfg['Iteration']).values[0]
    ideal_avx_cfg['Implementation'] = 'Dynamic'
    ideal_avx_df = pd.DataFrame(ideal_avx_cfg, index=[0])

    # Create a dataframe for all variations
    all_cfgs = [ref_sys_cfg, static_sys_cfg, ideal_cfg_df]
    all_avx_cfgs = [ref_avx_sys_cfg, static_avx_sys_cfg, ideal_avx_df]
    frames = pd.concat(all_cfgs)
    avx_frames = pd.concat(all_avx_cfgs)

    #for index, row in dyn_cfg.iterrows():
        #print index, row

    #print ref_sys_cfg
    #print ideal_cfg
    #print static_sys_cfg
    print dyn_cfg.transpose()

    #print '=============='
    #print ref_avx_sys_cfg
    #print static_avx_sys_cfg
    #print ideal_avx_cfg

    # Compute the trade-offs
    #print 'Trade-off'
    tradeoff(args['objective'], ref_sys_cfg, static_sys_cfg, ideal_cfg)

    #print '=============='
    #print 'Trade-off AVX'
    #tradeoff(args['objective'], ref_avx_sys_cfg, static_avx_sys_cfg, ideal_avx_cfg)

    # Plot
    pcfg = plot_cfg(args['objective'].lower(), objectives)
    barplot(args['objective'], args['scale'], frames, avx_frames, pcfg)


if __name__ == '__main__':
    main()
