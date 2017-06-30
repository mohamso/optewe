#include "nemi.h"

void process_kernel_energy(hdeem::connection& con, std::vector<kernel> kernel_energy_data) {

  auto globals = con.get_global();
  auto const& status = con.get_status();

  auto hdeem_start = status.start_time_blade.tv_sec + (double) status.start_time_blade.tv_nsec * 1e-9f;
  auto hdeem_stop = status.stop_time_blade.tv_sec + (double) status.stop_time_blade.tv_nsec * 1e-9f;
  auto hdeem_duration = hdeem_stop - hdeem_start;

  auto const& blade_data = globals.get_single_sensor_data(hdeem::sensor_id::blade(0));
  double sample_rate = blade_data.size() / hdeem_duration;

  for (auto const& kernel_data: kernel_energy_data) {

    double kernel_duration_ms = kernel_data.runtime * 1e-6f;
    double kernel_tstart_sec = kernel_data.timestamp/1000.0f;

    auto start_offset = (uint64_t) floor((kernel_tstart_sec - hdeem_start) * sample_rate);
    auto stop_offset = (uint64_t) ceil(start_offset+kernel_duration_ms);

    double avg_energy = kernel_energy(blade_data, kernel_data, start_offset, stop_offset);

    std::cout << kernel_data.name << "," << kernel_data.iteration << "," << std::fixed << kernel_duration_ms
              << "," << std::fixed << avg_energy << std::endl;
  }
}

double kernel_energy(const hdeem::detail::single_sensor_data& blade_data, struct kernel kernel_data,
                     uint64_t start_offset, uint64_t stop_offset) {

  uint64_t counter = 0;
  double value = 0;

  for (auto const& timeval : blade_data) {
    if (timeval.first >= start_offset && timeval.first <= stop_offset) {
      value += timeval.second;
      counter++;
    }
  }

  double avg_power = value / counter;
  double avg_energy = avg_power * (kernel_data.runtime*1e-9f);

  return avg_energy;
}
