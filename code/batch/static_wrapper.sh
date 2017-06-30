#!/bin/bash
#
# Author: Mohammed Sourouri <mohammed.sourouri@ntnu.no>

if [ "$#" -ne 3 ]; then
    echo "Usage: size niterations output_folder"
    exit 1
fi

size=$1
niterations=$2
folder=$3

directory=$SLURM_SUBMIT_DIR
sh ${directory}/set-governor.sh

mkdir -p ${folder}

KERNELS=(DXF DZB DYB CVX \
         DYF DZB2 DXB CVY \
         DZF DXB2 DYB2 CVZ \
         DZB3 DXB3 DYB3 CSXXSYYSZZ \
         DYF2 DXF2 CSXY \
         DZF2 DYF3 CSYZ \
         DXF3 DZF3 CSXZ)

INSTRUMENTATION="-DSTATIC_DVFS -DHDEEM "
for kernels in "${KERNELS[@]}"; do
    INSTRUMENTATION+="-D${kernels}_HDEEM "
done

sh ${directory}/compile.sh "${INSTRUMENTATION}"

for core in {25..12}; do
    for uncore in {30..12}; do
        for thread in {8..8}; do
        file="${size}_${niterations}_c${core}_u${uncore}_t${thread}"
        export OMP_NUM_THREADS=$thread
        export KMP_AFFINITY=granularity=fine,compact
        cd ${directory}/../bin/
        ./optewe-mp ${size} ${size} ${size} ${niterations} ${core} ${uncore} 2 > ../batch/${folder}/${file}
        cd ${directory}
        done
    done
done

