/* Original author: Espen B. Raknes <espen.raknes@ntnu.no>
 * Modified by: Mohammed Sourouri <mohammed.sourouri@ntnu.no>
 * Date: December 16, 2016
*/

#include "differentiators.h"

void dx_forward(real* to, const real* __restrict__ from, const int nx, const int ny, const int nz, const real scale, const int nthreads) {

  zero_data(to, nx, ny, nz);

  #pragma omp parallel for num_threads(nthreads)
  for (int k = half_length; k < nz - half_length; k++) {
    for (int j = half_length; j < ny - half_length; j++) {
      for (int i = half_length; i < nx - half_length; i++) {

        for (int l = 0; l < half_length; l++) {
          to[idx(nx, ny, i, j, k)] += W[l] * (from[idx(nx, ny, i+l+1, j, k)] - from[idx(nx, ny, i-l, j, k)]);
        }

        to[idx(nx, ny, i, j, k)] *= scale;
      }
    }
  }
}

void dx_backward(real* to, const real* __restrict__ from, const int nx, const int ny, const int nz, const real scale, const int nthreads) {

  zero_data(to, nx, ny, nz);

  #pragma omp parallel for num_threads(nthreads)
  for (int k = half_length; k < nz - half_length; k++) {
    for (int j = half_length; j < ny - half_length; j++) {
      for (int i = half_length; i < nx - half_length; i++) {

        for (int l = 0; l < half_length; l++) {
          to[idx(nx, ny, i, j, k)] += W[l] * (from[idx(nx, ny, i+l, j, k)] - from[idx(nx, ny, i-l-1, j, k)]);
        }

        to[idx(nx, ny, i, j, k)] *= scale;
      }
    }
  }
}

void dy_forward(real* to, const real* __restrict__ from, const int nx, const int ny, const int nz, const real scale, const int nthreads) {

  zero_data(to, nx, ny, nz);

  #pragma omp parallel for num_threads(nthreads)
  for (int k = half_length; k < nz - half_length; k++) {
    for (int j = half_length; j < ny - half_length; j++) {
      for (int i = half_length; i < nx - half_length; i++) {

        for (int l = 0; l < half_length; l++) {
          to[idx(nx, ny, i, j, k)] += W[l] * (from[idx(nx, ny, i, j+l+1, k)] - from[idx(nx, ny, i, j-l, k)]);
        }

        to[idx(nx, ny, i, j, k)] *= scale;
      }
    }
  }
}

void dy_backward(real* to, const real* __restrict__ from, const int nx, const int ny, const int nz, const real scale, const int nthreads) {

  zero_data(to, nx, ny, nz);

  #pragma omp parallel for num_threads(nthreads)
  for (int k = half_length; k < nz - half_length; k++) {
    for (int j = half_length; j < ny - half_length; j++) {
      for (int i = half_length; i < nx - half_length; i++) {

        for (int l = 0; l < half_length; l++) {
          to[idx(nx, ny, i, j, k)] += W[l] * (from[idx(nx, ny, i, j+l, k)] - from[idx(nx, ny, i, j-l-1, k)]);
        }

        to[idx(nx, ny, i, j, k)] *= scale;
      }
    }
  }
}

void dz_forward(real* to, const real* __restrict__ from, const int nx, const int ny, const int nz, const real scale, const int nthreads) {

  zero_data(to, nx, ny, nz);

  #pragma omp parallel for num_threads(nthreads)
  for (int k = half_length; k < nz - half_length; k++) {
    for (int j = half_length; j < ny - half_length; j++) {
      for (int i = half_length; i < nx - half_length; i++) {

        for (int l = 0; l < half_length; l++) {
          to[idx(nx, ny, i, j, k)] += W[l] * (from[idx(nx, ny, i, j, k+l+1)] - from[idx(nx, ny, i, j, k-l)]);
        }

        to[idx(nx, ny, i, j, k)] *= scale;
      }
    }
  }
}

void dz_backward(real* to, const real* __restrict__ from, const int nx, const int ny, const int nz, const real scale, const int nthreads) {

  zero_data(to, nx, ny, nz);

  #pragma omp parallel for num_threads(nthreads)
  for (int k = half_length; k < nz - half_length; k++) {
    for (int j = half_length; j < ny - half_length; j++) {
      for (int i = half_length; i < nx - half_length; i++) {

        for (int l = 0; l < half_length; l++) {
          to[idx(nx, ny, i, j, k)] += W[l] * (from[idx(nx, ny, i, j, k+l)] - from[idx(nx, ny, i, j, k-l-1)]);
        }

        to[idx(nx, ny, i, j, k)] *= scale;

      }
    }
  }
}

