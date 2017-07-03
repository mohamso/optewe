#!/bin/bash
#
# Author: Nico Reissmann <nico.reissmann@ntnu.no>
# Modified by: Mohammed Sourouri <mohammed.sourouri@ntnu.no>

# Print out hostname
hostname

size=512
niterations=5

KERNELS=(DXF DZB DYB CVX \
         DYF DZB2 DXB CVY \
         DZF DXB2 DYB2 CVZ \
         DZB3 DXB3 DYB3 CSXXSYYSZZ \
         DYF2 DXF2 CSXY \
         DZF2 DYF3 CSYZ \
         DXF3 DZF3 CSXZ)

folder=$size"_"${kernel}"_results"
mkdir -p ${folder}

for kernel in "${KERNELS[@]}"; do
    folder=$size"_"${kernel}"_results"
    mkdir -p ${folder}
    for core in {25..12}; do
        for uncore in {25..12}; do
            for thread in {24..2}; do
                INSTRUMENTATION="-D${kernel}_TIME -D${kernel}_DVFS -D${kernel}_CORE=${core} -D${kernel}_UNCORE=${uncore} -DHDEEM"
                sh $SLURM_SUBMIT_DIR/compile.sh "${INSTRUMENTATION}"
                file="${kernel}_${size}_${niterations}_c${core}_u${uncore}_${thread}"
                 export OMP_NUM_THREADS=${thread}
                 cd $SLURM_SUBMIT_DIR/../bin/
                 ./optewe-mp ${size} ${size} ${size} ${niterations} ${thread} > "$SLURM_SUBMIT_DIR/${folder}/${file}"
                 cd $SLURM_SUBMIT_DIR
            done
        done
    done
done
