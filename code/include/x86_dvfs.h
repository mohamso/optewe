/* Author: Mohammed Sourouri <mohammed.sourouri@ntnu.no>
 * Date: October 12, 2016
 * Updated: February 14, 2017
 */

#ifndef X86_DVFS_X86_DVFS_H
#define X86_DVFS_X86_DVFS_H

#include <cstdlib>
#include <iostream>
#include <iomanip>

#include "x86_adapt.h"

void dvfs_init();
void dvfs_finalize();

int get_target_pstate(x86_adapt_device_type core_type);
int get_perf_status_pstate(x86_adapt_device_type core_type);

int get_uncore_min_ratio(x86_adapt_device_type uncore_type);
int get_uncore_max_ratio(x86_adapt_device_type uncore_type);
int get_uncore_current_ratio(x86_adapt_device_type uncore_type);

float get_avg_core_freq(x86_adapt_device_type core_type, int num_cores, int core_freq_id);
float* get_uncore_freq(x86_adapt_device_type uncore_type, int numa_nodes, int uncore_freq_id, float* uncore_freqs);

void set_all_core_freq(x86_adapt_device_type core_type, int fd, int pstate_idx, uint64_t core_freq);
void set_core_freq(x86_adapt_device_type core_type, int num_cores, int pstate_idx, uint64_t core_freq);
void set_uncore_freq(x86_adapt_device_type uncore_type, int numa_nodes, int idx_min, int idx_max, uint64_t uncore_freq);

void print_core_freq(float avg_cpu_freq);
void print_uncore_freq(int numa_nodes, const float* uncore_freqs);

#endif //X86_DVFS_X86_DVFS_H
