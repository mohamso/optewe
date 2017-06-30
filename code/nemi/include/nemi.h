/* Author: Mohammed Sourouri <mohammed.sourouri@ntnu.no>
 * Date: March 16, 2017
 */

#include <hdeem_cxx.hpp>
#include <cstdint>
#include <chrono>

#ifndef NTNU_ENERGY_MEASUREMENT_INFRASTRUCTURE
#define NTNU_ENERGY_MEASUREMENT_INFRASTRUCTURE

struct kernel {
  std::string name;
  int iteration;
  double timestamp;
  double runtime;

  kernel(const std::string& name, int iteration, double timestamp, double runtime) :
      name(name),
      iteration(iteration),
      timestamp(timestamp),
      runtime(runtime) {}
};

void process_kernel_energy(hdeem::connection& con, std::vector<kernel> kernel_energy_data);
double kernel_energy(const hdeem::detail::single_sensor_data& blade_data, struct kernel kernel_data,
                     uint64_t start_offset, uint64_t stop_offset);

#endif // NTNU_ENERGY_MEASUREMENT_INFRASTRUCTURE
