#!/bin/bash
#
# Author: Nico Reissmann <nico.reissmann@ntnu.no>
# Modified by: Mohammed Sourouri <mohammed.sourouri@ntnu.no>

if [ "$#" -ne 3 ]; then
    echo "Usage: <script> #size #iterations #threads"
    exit 1
fi

export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
export OMP_NUM_THREADS=$3
export KMP_AFFINITY="granularity=core,compact"

for n in {0..23}; do
    echo -n "userspace" > /sys/devices/system/cpu/cpu$n/cpufreq/scaling_governor
done

cd ../bin
./optewe-mp $1 $1 $1 $2 2

