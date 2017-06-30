/* Original Author: Espen B. Raknes <espen.raknes@ntnu.no>
 * Modified by: Mohammed Sourouri <mohammed.sourouri@ntnu.no>
 * Date: December 16, 2016
 * Comment: Takes one time step forward in time for the 3D elastic wave equation.
 * The spatial differentiators use a half grid width of six grid cells.
 */

#include "step_forward.h"

void compute_vx(real* vx, const real* __restrict__ rho, const real* __restrict__ del1,
                const real* __restrict__ del2, const real* __restrict__ del3, const real dt,
                const int nx_ghost, const int ny_ghost, const int nz_ghost) {

  #pragma omp parallel for
  for (int k = 0; k < nz_ghost; k++) {
    for (int j = 0; j < ny_ghost; j++) {
      for (int i = 0; i < nx_ghost-1; i++) {
        vx[idx(nx_ghost, ny_ghost, i, j, k)] += dt * (2.0 / (rho[idx(nx_ghost, ny_ghost, i, j, k)]
            + rho[idx(nx_ghost, ny_ghost, i+1, j, k)])) * (del1[idx(nx_ghost, ny_ghost, i, j, k)]
            + del2[idx(nx_ghost, ny_ghost, i, j, k)] + del3[idx(nx_ghost, ny_ghost, i, j, k)]);
      }
    }
  }
}

void compute_vy(real* vy, const real* __restrict__ rho, const real* __restrict__ del1,
                const real* __restrict__ del2, const real* __restrict__ del3, const real dt,
                const int nx_ghost, const int ny_ghost, const int nz_ghost) {

  #pragma omp parallel for
  for (int k = 0; k < nz_ghost; k++) {
    for (int j = 0; j < ny_ghost - 1; j++) {
      for (int i = 0; i < nx_ghost; i++) {
        vy[idx(nx_ghost, ny_ghost, i, j, k)] += dt * (2.0 / (rho[idx(nx_ghost, ny_ghost, i, j, k)]
            + rho[idx(nx_ghost, ny_ghost, i, j+1, k)])) * (del1[idx(nx_ghost, ny_ghost, i, j, k)]
            + del2[idx(nx_ghost, ny_ghost, i, j, k)] + del3[idx(nx_ghost, ny_ghost, i, j, k)]);
      }
    }
  }
}


void compute_vz(real* vz, const real* __restrict__ rho, const real* __restrict__ del1,
                const real* __restrict__ del2, const real* __restrict__ del3, const real dt,
                const int nx_ghost, const int ny_ghost, const int nz_ghost) {

  #pragma omp parallel for
  for (int k = 0; k < nz_ghost - 1; k++) {
    for (int j = 0; j < ny_ghost; j++) {
      for (int i = 0; i < nx_ghost; i++) {
        vz[idx(nx_ghost, ny_ghost, i, j, k)] += dt * (2.0 / (rho[idx(nx_ghost, ny_ghost, i, j, k)]
            + rho[idx(nx_ghost, ny_ghost, i, j, k+1)])) * (del1[idx(nx_ghost, ny_ghost, i, j, k)]
            + del2[idx(nx_ghost, ny_ghost, i, j, k)] + del3[idx(nx_ghost, ny_ghost, i, j, k)]);
      }
    }
  }
}


void compute_sxy(real* sxy, const real* __restrict__ mu, const real* __restrict__ del1,
                 const real* __restrict__ del2, const real dt,
                 const int nx_ghost, const int ny_ghost, const int nz_ghost) {

  #pragma omp parallel for
  for (int k = 0; k < nz_ghost; k++) {
    for (int j = 0; j < ny_ghost - 1; j++) {
      for (int i = 0; i < nx_ghost - 1; i++) {
        sxy[idx(nx_ghost, ny_ghost, i, j, k)] += dt * (mu[idx(nx_ghost, ny_ghost, i, j, k)]
            + mu[idx(nx_ghost, ny_ghost, i + 1, j, k)] + mu[idx(nx_ghost, ny_ghost, i, j + 1, k)]
            + mu[idx(nx_ghost, ny_ghost, i + 1, j + 1, k)]) * 0.25 * (del1[idx(nx_ghost, ny_ghost, i, j, k)]
            + del2[idx(nx_ghost, ny_ghost, i, j, k)]);
      }
    }
  }
}

void compute_syz(real* syz, const real* __restrict__ mu, const real* __restrict__ del1,
                 const real* __restrict__ del2, const real dt,
                 const int nx_ghost, const int ny_ghost, const int nz_ghost) {

  #pragma omp parallel for
  for (int k = 0; k < nz_ghost - 1; k++) {
    for (int j = 0; j < ny_ghost - 1; j++) {
      for (int i = 0; i < nx_ghost; i++) {
        syz[idx(nx_ghost, ny_ghost, i, j, k)] += dt * (mu[idx(nx_ghost, ny_ghost, i, j, k)]
            + mu[idx(nx_ghost, ny_ghost, i, j+1, k)] + mu[idx(nx_ghost, ny_ghost, i, j, k+1)]
            + mu[idx(nx_ghost, ny_ghost, i, j+1, k+1)]) * 0.25 * (del1[idx(nx_ghost, ny_ghost, i, j, k)]
            + del2[idx(nx_ghost, ny_ghost, i, j, k)]);
      }
    }
  }
}

void compute_sxz(real* sxz, const real* __restrict__ mu, const real* __restrict__ del1,
                 const real* __restrict__ del2, const real dt,
                 const int nx_ghost, const int ny_ghost, const int nz_ghost) {

  #pragma omp parallel for
  for (int k = 0; k < nz_ghost - 1; k++) {
    for (int j = 0; j < ny_ghost; j++) {
      for (int i = 0; i < nx_ghost - 1; i++) {
        sxz[idx(nx_ghost, ny_ghost, i, j, k)] += dt * (mu[idx(nx_ghost, ny_ghost, i, j, k)]
            + mu[idx(nx_ghost, ny_ghost, i+1, j, k)] + mu[idx(nx_ghost, ny_ghost, i, j+1, k)]
            + mu[idx(nx_ghost, ny_ghost, i+1, j, k+1)]) * 0.25 * (del1[idx(nx_ghost, ny_ghost, i, j, k)]
            + del2[idx(nx_ghost, ny_ghost, i, j, k)]);
      }
    }
  }
}

void compute_sxx_syy_szz(real* sxx, real* syy, real* szz, const real* __restrict__ del1,
                         const real* __restrict__ del2, const real* __restrict__ del3,
                         const real* __restrict__ lambda, const real* __restrict__ mu,  const real dt,
                         const int nx_ghost, const int ny_ghost, const int nz_ghost) {

  #pragma omp parallel for
  for (int k = 0; k < nz_ghost; k++) {
    for (int j = 0; j < ny_ghost; j++) {
      for (int i = 0; i < nx_ghost; i++) {
        sxx[idx(nx_ghost, ny_ghost, i, j, k)] += dt * ((lambda[idx(nx_ghost, ny_ghost, i, j, k)]
            + 2.0 * mu[idx(nx_ghost, ny_ghost, i, j, k)])
            * del2[idx(nx_ghost, ny_ghost, i, j, k)]
            + lambda[idx(nx_ghost, ny_ghost, i, j, k)] * (del1[idx(nx_ghost, ny_ghost, i, j, k)]
                + del3[idx(nx_ghost, ny_ghost, i, j, k)]));

        syy[idx(nx_ghost, ny_ghost, i, j, k)] += dt * ((lambda[idx(nx_ghost, ny_ghost, i, j, k)]
            + 2.0 * mu[idx(nx_ghost, ny_ghost, i, j, k)])
            * del3[idx(nx_ghost, ny_ghost, i, j, k)]
            + lambda[idx(nx_ghost, ny_ghost, i, j, k)] * (del1[idx(nx_ghost, ny_ghost, i, j, k)]
                + del2[idx(nx_ghost, ny_ghost, i, j, k)]));

        szz[idx(nx_ghost, ny_ghost, i, j, k)] += dt * ((lambda[idx(nx_ghost, ny_ghost, i, j, k)]
            + 2.0 * mu[idx(nx_ghost, ny_ghost, i, j, k)])
            * del1[idx(nx_ghost, ny_ghost, i, j, k)]
            + lambda[idx(nx_ghost, ny_ghost, i, j, k)] * (del2[idx(nx_ghost, ny_ghost, i, j, k)]
                + del3[idx(nx_ghost, ny_ghost, i, j, k)]));
      }
    }
  }
}
