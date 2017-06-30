/* Author: Mohammed Sourouri <mohammed.sourouri@ntnu.no>
 * Date: January 16, 2012
 * Updated: September 3, 2016
 * Comment: A series of print functions used in simulation codes.
 */

#include "print.h"

void print_application_info(std::string app_name, const int source_type, const int Nx, const int Ny, const int Nz, const int num_iterations) {

  std::cout << "#========================= " << app_name << " =========================\n";
  if (source_type == 1) {
    std::cout << "#Source type                                  :  Stress monopole" << std::endl;
  }
  else if (source_type == 2) {
    std::cout << "#Source type                                  :  Force monopole" << std::endl;
  }
  else if (source_type == 3) {
    std::cout << "#Source type                                  :  Force dipole" << std::endl;
  }

  std::cout << "#Grid Size                                    :  " << Nx << " x " << Ny << " x " << Nz << std::endl;
  std::cout << "#Iterations                                   :  " << num_iterations << std::endl;
}

void print_omp_info(const unsigned int num_threads) {
  std::cout << "#Number of threads                            :  " << num_threads << std::endl;
}

void print_perf_summary(const double mlups, const double compute_timer) {
  std::cout << "#Compute time                                 :  " << compute_timer << std::endl;
  std::cout << "#Total effective MLUPS                        :  " << mlups << std::endl;
  std::cout << "#===================================================================\n";
}

void print_3D(const real* __restrict__ buffer, const int Nx, const int Ny, const int Nz) {
  for (int k = 0; k < Nz; k++) {
    for (int j = 0; j < Ny; j++) {
      for (int i = 0; i < Nx; i++) {
        printf("%8.2f", buffer[idx(Nx, Ny, i, j, k)]);
      }
      printf("\n");
    }
    printf("\n");
  }
}

void print_2D(const real* __restrict__ buffer, const int Nx, const int Ny) {
  for (int j = 0; j < Ny; j++) {
    for (int i = 0; i < Nx; i++) {
      int idx = i + j * (Nx + 2);
      printf("%8.2f", buffer[idx]);
    }
    printf("\n");
  }
  printf("\n");
}

