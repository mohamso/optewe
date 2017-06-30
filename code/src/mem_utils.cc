/* Author: Mohammed Sourouri <mohammed.sourouri@ntnu.no>
 * Date: February 24, 2016
 * Comment: A set of memory utilities for touching data.
*/

#include "mem_utils.h"

void zero_data(real* buffer, const int nx, const int ny, const int nz) {

#pragma omp parallel for
  for (int k = 0; k < nz; k++) {
    for (int j = 0; j < ny; j++) {
      for (int i = 0; i < nx; i++) {
        buffer[idx(nx, ny, i, j, k)] = 0.f;
      }
    }
  }
}


