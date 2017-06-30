/* Author: Mohammed Sourouri <mohammed.sourouri@ntnu.no>
 * Comment: A utility to change the core and uncore frequency on Taurus.
 * Date: October 12, 2016
 * Updated: February 14, 2017
 */

#include "x86_dvfs.h"

void dvfs_init() {

  if (x86_adapt_init()) {
    std::cerr << "Initialization failed\n";
    exit(1);
  }
}

void dvfs_finalize() {
  x86_adapt_finalize();
}

int get_target_pstate(x86_adapt_device_type core_type) {

  int pstate_index = x86_adapt_lookup_ci_name(core_type, "Intel_Target_PState");

  if (pstate_index < 0) {
    std::cerr << "Could not find item Intel_Target_PState\n";
    exit(1);
  }

  return pstate_index;
}

int get_uncore_min_ratio(x86_adapt_device_type uncore_type) {

  int index_min = x86_adapt_lookup_ci_name(uncore_type, "Intel_UNCORE_MIN_RATIO");

  if (index_min < 0) {
    std::cerr << "Could not find item Intel_UNCORE_MIN_RATIO\n";
    exit(1);
  }

  return index_min;
}

int get_uncore_max_ratio(x86_adapt_device_type uncore_type) {

  int index_max = x86_adapt_lookup_ci_name(uncore_type, "Intel_UNCORE_MAX_RATIO");

  if (index_max < 0) {
    std::cerr << "Could not find item Intel_UNCORE_MAX_RATIO\n";
    exit(1);
  }

  return index_max;
}

int get_perf_status_pstate(x86_adapt_device_type core_type) {

  int core_freq_id = x86_adapt_lookup_ci_name(core_type, "Intel_PERF_STATUS_Current_PState");

  if (core_freq_id < 0) { std::cerr << "Could not find Intel_PERF_STATUS_Current_PState\n"; }

  return core_freq_id;
}

int get_uncore_current_ratio(x86_adapt_device_type uncore_type) {

  int uncore_frequency_id = x86_adapt_lookup_ci_name(uncore_type, "Intel_UNCORE_CURRENT_RATIO");

  if (uncore_frequency_id < 0) {
    std::cerr << "Could not find Intel_UNCORE_CURRENT_RATIO\n";
  }

  return uncore_frequency_id;
}

float get_avg_core_freq(x86_adapt_device_type core_type, int num_cores, int core_freq_id) {

  uint64_t result;

  float avg_cpu_freq = 0.;

  for (int cpu = 0; cpu < num_cores; cpu++) {
    int fd = x86_adapt_get_device(core_type, cpu);
    if (fd > 0) {
      if (x86_adapt_get_setting(fd, core_freq_id, &result) == 8) {
        avg_cpu_freq += (result / 256.0) / 10.0f;
      } else {
        std::cerr << "Could not get the core frequency for core #" << cpu << std::endl;
      }
    } else { std::cerr << "Open failed\n"; }
  }

  return avg_cpu_freq;
}

float* get_uncore_freq(x86_adapt_device_type uncore_type, int numa_nodes, int uncore_freq_id, float* uncore_freqs) {

  uint64_t result;

  for (int numa_node = 0; numa_node < numa_nodes; numa_node++) {
    int fd = x86_adapt_get_device(uncore_type, numa_node);
    if (fd > 0) {
      if (x86_adapt_get_setting(fd, uncore_freq_id, &result) == 8) {
        uncore_freqs[numa_node] = result/10.0f;
      } else { std::cerr << "Could not get the uncore frequency for NUMA node #" << numa_node << std::endl; }
    } else { std::cerr << "Failed to get a file descriptor for X86_ADAPT_DIE\n"; }
  }

  return uncore_freqs;
}

void set_core_freq(x86_adapt_device_type core_type, int num_cores, int pstate_idx, uint64_t core_freq) {

  for (int core = 0; core < num_cores; core++) {
    int fd = x86_adapt_get_device(core_type, core);
    if (fd > 0) {
      if (x86_adapt_set_setting(fd, pstate_idx, (core_freq << 8)) != 8) {
        std::cerr << "Could not change the core CPU frequency (Intel_Target_PState)\n";
      }
    } else {
      std::cerr << "Failed to get a file descriptor for X86_ADAPT_CPU\n";
      exit(1);
    }
  }
}

void set_all_core_freq(x86_adapt_device_type core_type, int fd, int pstate_idx, uint64_t core_freq) {

  if (fd > 0) {
    if (x86_adapt_set_setting(fd, pstate_idx, (core_freq << 8)) != 8) {
      std::cerr << "Could not change the core CPU frequency (Intel_Target_PState)\n";
    }
  } else {
    std::cerr << "Failed to get a file descriptor for X86_ADAPT_CPU\n";
    exit(1);
  }
}

void set_uncore_freq(x86_adapt_device_type uncore_type, int numa_nodes, int idx_min, int idx_max, uint64_t uncore_freq) {

  for (int numa_node = 0; numa_node < numa_nodes; numa_node++) {
    int fd = x86_adapt_get_device(uncore_type, numa_node);
    if (fd > 0) {
      if (x86_adapt_set_setting(fd, idx_min, uncore_freq) != 8) {
        std::cerr << "Could not set the Intel_UNCORE_MIN_RATIO\n";
      }
      if (x86_adapt_set_setting(fd, idx_max, uncore_freq) != 8) {
        std::cerr << "Could not set the Intel_UNCORE_MAX_RATIO\n";
      }
    } else {
      std::cerr << "Failed to get a file descriptor for X86_ADAPT_DIE\n";
      exit(1);
    }
  }
}

void print_core_freq(float avg_cpu_freq) {
  std::cout << "#===================================================================\n";
  std::cout << "#Average CPU frequency                        : " << std::fixed << std::setprecision(2)
            << avg_cpu_freq/24.0f << " GHz\n";
}

void print_uncore_freq(int numa_nodes, const float* uncore_freqs) {

  for (int numa_node = 0; numa_node < numa_nodes; numa_node++) {
    std::cout << "#Uncore frequency for NUMA node #" << numa_node << "            : " << std::fixed
              << std::setprecision(2) << uncore_freqs[numa_node] << " GHz\n";
  }
}
