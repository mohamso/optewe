#!/bin/bash
#
# Author: Mohammed Sourouri <mohammed.sourouri@ntnu.no>

for n in {0..23}; do
    echo -n "userspace" > /sys/devices/system/cpu/cpu$n/cpufreq/scaling_governor
done

