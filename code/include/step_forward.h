/* Original Author: Espen B. Raknes <espen.raknes@ntnu.no>
 * Modified by: Mohammed Sourouri <mohammed.sourouri@ntnu.no>
 * Date: September 11, 2016
 */

#ifndef STEPFORWARD_H
#define STEPFORWARD_H

#include "fd3d.h"
#include "model3d.h"

void compute_vx(real* vx, const real* __restrict__ rho, const real* __restrict__ del1,
                const real* __restrict__ del2, const real* __restrict__ del3, const real dt,
                const int nx_ghost, const int ny_ghost, const int nz_ghost);


void compute_vy(real* vy, const real* __restrict__ rho, const real* __restrict__ del1,
                const real* __restrict__ del2, const real* __restrict__ del3, const real dt,
                const int nx_ghost, const int ny_ghost, const int nz_ghost);

void compute_vz(real* vz, const real* __restrict__ rho, const real* __restrict__ del1,
                const real* __restrict__ del2, const real* __restrict__ del3, const real dt,
                const int nx_ghost, const int ny_ghost, const int nz_ghost);

void compute_sxy(real* sxy, const real* __restrict__ mu, const real* __restrict__ del1,
                 const real* __restrict__ del2, const real dt,
                 const int nx_ghost, const int ny_ghost, const int nz_ghost);

void compute_syz(real* syz, const real* __restrict__ mu, const real* __restrict__ del1,
                 const real* __restrict__ del2, const real dt,
                 const int nx_ghost, const int ny_ghost, const int nz_ghost);

void compute_sxz(real* sxz, const real* __restrict__ mu, const real* __restrict__ del1,
                 const real* __restrict__ del2, const real dt,
                 const int nx_ghost, const int ny_ghost, const int nz_ghost);

void compute_sxx_syy_szz(real* sxx, real* syy, real* szz, const real* __restrict__ del1,
                         const real* __restrict__ del2, const real* __restrict__ del3,
                         const real* __restrict__ lambda, const real* __restrict__ mu,  const real dt,
                         const int nx_ghost, const int ny_ghost, const int nz_ghost);
#endif // STEPFORWARD_H