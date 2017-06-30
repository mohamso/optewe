#!/usr/bin/env python
# Author: Mohammed Sourouri <mohammed.sourouri@ntnu.no>
# Date: January 26, 2017
# Updated: March 18, 2017

import re, os
import pandas as pd
import numpy as np
from utils import edp, ed2p, mlups, canonize, implementation
from collections import defaultdict


def kname(s):

    m = re.search('(^[^,]*)', s)
    return str(m.group(1))


def size(s):
    m = re.search('(^\d+)', s)
    return str(m.group(1))


def niters(s):
    m = re.search('_(\d+)_c', s)
    return int(m.group(1))


def niter(s):
    m = re.search(',(\d+),', s)
    return int(m.group(1))


def energy(l):
    m = re.search('\d+(\.\d+)$', l)
    return float(m.group(0))


def ctime(l):
    m = re.search('time\s+:\s+(\d+(\.\d+)?)', l)
    return float(m.group(1))


def ktime(l):

    m = re.search(',(\d+(\.\d+)),', l)
    return float(m.group(1))


def nthreads(fname):
    m = re.search('_t(\d+(\.\d+)?)', fname)
    return int(m.group(1))


def uncoref(fname):
    m = re.search('_u(.+?)_', fname)
    return float(m.group(1))/10.0


def coref(fname):
    m = re.search('_c(.+?)_', fname)
    return float(m.group(1))/10.0


def best_static_cfgs(objective, static_cfg):

    cfg_dict = {'mlups': static_cfg.loc[static_cfg[canonize(objective)].idxmax()],
                'runtime': static_cfg.loc[static_cfg[canonize(objective)].idxmin()],
                'edp': static_cfg.loc[static_cfg[canonize(objective)].idxmin()],
                'edp2': static_cfg.loc[static_cfg[canonize(objective)].idxmin()],
                'energy': static_cfg.loc[static_cfg[canonize(objective)].idxmin()]
                }

    return cfg_dict


def best_static_cfg(objective, static_cfg):

    sorted_df = static_cfg.sort_values(by=[canonize(objective)])
    return sorted_df.head(1)


def static_sys_cfgs(directory):

    msize = []
    iters = []
    corefs = []
    ucorefs = []
    threads = []
    runtime = []
    benergy = []
    impl = []

    for filename in os.listdir(directory):
        msize.append(size(filename))
        iters.append(niters(filename))
        threads.append(nthreads(filename))
        corefs.append(coref(filename))
        ucorefs.append(uncoref(filename))
        impl.append(implementation(directory))
        with open(directory + '/' + filename, 'r') as open_file:
            kenergy = []
            for line in open_file:
                if line.startswith('#'):
                    if line.startswith('#C'):
                        runtime.append(ctime(line))
                    continue
                kenergy.append(energy(line))
            benergy.append(np.sum(np.asarray(kenergy)))

    mlupss = [mlups(t, s, i) for t, s, i in zip(runtime, msize, iters)]
    edps = [edp(e, t) for e, t in zip(benergy, runtime)]
    ed2ps = [ed2p(e, t) for e, t in zip(benergy, runtime)]

    labels = ['Size', 'Iteration', 'Core', 'Uncore', 'Threads', 'Runtime', 'Energy',
              'EDP', 'ED2P', 'MLUPS', 'Implementation']

    static_cfg_data = [msize, iters, corefs, ucorefs, threads, runtime, benergy, edps, ed2ps, mlupss, impl]
    df = pd.DataFrame.from_records(static_cfg_data, index=labels)
    return df.transpose()


def ideal_dyn_cfg(data):

    dft = data.transpose()
    runtime_seconds = (dft['Runtime'].sum()*dft['Iteration'].min())/1000.0
    blade_energy = dft['Energy'].sum()*dft['Iteration'].min()
    ideal_cfg = {canonize('runtime'): runtime_seconds, canonize('energy'): blade_energy,
                 canonize('edp'): edp(blade_energy, runtime_seconds),
                 canonize('ed2p'): ed2p(blade_energy, runtime_seconds)}

    return ideal_cfg


def best_dynamic_sys_cfg(objective, data):

    sorted_df = data.sort_values(by=[canonize(objective)])

    kname_dict = {}
    for index, value in sorted_df.iterrows():

        if not value['Kernel Name'] in kname_dict:
            kname_dict[value['Kernel Name']] = (value['Size'], value['Iteration'], value['Core'], value['Uncore'],
                                                value['Threads'], value['Runtime'], value['Energy'], value['EDP'],
                                                value['ED2P'])

    labels = ['Size', 'Iteration', 'Core', 'Uncore', 'Threads', 'Runtime', 'Energy', 'EDP', 'ED2P']
    df = pd.DataFrame.from_records(kname_dict, index=labels)
    return df


def dynamic_sys_cfgs(directory):

    results = []

    for filename in os.listdir(directory):
        with open(directory + '/' + filename, 'r') as open_file:
            ktimes = defaultdict(list)
            kenergy = defaultdict(list)
            for line in open_file:
                if line.startswith('#'):
                    continue
                ktimes[kname(line)].append(ktime(line))
                kenergy[kname(line)].append(energy(line))

        for k, v in kenergy.iteritems():
            results.append((k, size(filename), niters(filename), coref(filename), uncoref(filename), nthreads(filename), np.mean(ktimes[k]),
                            np.mean(kenergy[k]), edp(np.mean(kenergy[k]), np.mean(ktimes[k])),
                            ed2p(np.mean(kenergy[k]), np.mean(ktimes[k]))))

    labels = ['Kernel Name', 'Size', 'Iteration', 'Core', 'Uncore', 'Threads', 'Runtime', 'Energy', 'EDP', 'ED2P']
    return pd.DataFrame.from_records(results, columns=labels)

