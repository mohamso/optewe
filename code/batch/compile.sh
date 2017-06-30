#!/bin/bash
#
# Original Author: Nico Reissmann <nico.reissmann@ntnu.no>
# Modified by: Mohammed Sourouri <mohammed.sourouri@ntnu.no>

args=""
for arg in "$@"
do
    args+="$arg "
done

module purge
module load intel/2016.2.181
module load hdeem

cd ..
make clean ; make -j24 INSTRUMENTATION="$args"

