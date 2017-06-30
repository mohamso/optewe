#!/usr/bin/env python
# Author: Mohammed Sourouri <mohammed.sourouri@ntnu.no>
# Date: March 18, 2017

import os
import re


def delete_ds_store(directory):
    for filename in os.listdir(directory):
        if filename == '.DS_Store':
            os.remove(directory+'/'+filename)


def edp(energy, runtime):
    return round(energy * runtime)


def ed2p(energy, runtime):
    return round(energy*(runtime**2), 2)


def mlups(time, size, iters):
    return (float(iters)*(float(size)**3)*1e-6) / float(time)


def canonize(objective):

    if len(objective) < 6:
        return objective.upper()
    else:
        return objective.title()


def exists(args, objectives):

    for arg in args:
        if str(arg).lower() in objectives:
            return True
    return False


def implementation(d):

    m = re.search('(_avx)', str(d))

    if m:
        if str(d).__contains__('static_tuning'):
            return 'AVX2 (ST)'
        else:
            return 'AVX2'
    else:
        if str(d).__contains__('reference'):
            return 'Reference'
        else:
            return 'Reference (ST)'