#!	/bin/bash

./create_512_edp.sh
./create_768_edp.sh
./create_1024_edp.sh
./create_512_ed2p.sh
./create_768_ed2p.sh
./create_1024_ed2p.sh
./create_512_energy.sh
./create_768_energy.sh
./create_1024_energy.sh

./create_kernel_times.sh
./create_avx_threads.sh
./create_reference_threads.sh
