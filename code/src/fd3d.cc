/* Original Author: Espen B. Raknes <espen.raknes@ntnu.no>
 * Modified by: Mohammed Sourouri <mohammed.sourouri@ntnu.no>
 * Date: September 10, 2016
 */

#include "fd3d.h"

// Initialization of the 3D finite difference structure
std::shared_ptr<fdm3d_t> fdm3d_setup(std::shared_ptr<dims_t> dims) {

  // Create wave structure
  std::shared_ptr<fdm3d_t> waves((fdm3d_t*) malloc(sizeof(fdm3d_t)), free_ptr());

  // Setup struct variables from wave input
  waves->ghost_border = dims->ghost_border;
  waves->nz = dims->nz;
  waves->nx = dims->nx;
  waves->ny = dims->ny;
  waves->dz = dims->dz;
  waves->dx = dims->dx;
  waves->dy = dims->dy;
  waves->dt = dims->dt;
  waves->nt = dims->nt;

  // Calculate coordinates with grid cells included
  waves->nx_ghost = waves->nx + 2 * waves->ghost_border;
  waves->ny_ghost = waves->ny + 2 * waves->ghost_border;
  waves->nz_ghost = waves->nz + 2 * waves->ghost_border;

  // Allocate arrays
  size_t num_bytes = sizeof(real) * ((waves->nx_ghost) * (waves->ny_ghost) * (waves->nz_ghost));

  // Stress fields
  waves->sxx = (real*) malloc(num_bytes);
  waves->syy = (real*) malloc(num_bytes);
  waves->szz = (real*) malloc(num_bytes);
  waves->sxy = (real*) malloc(num_bytes);
  waves->syz = (real*) malloc(num_bytes);
  waves->sxz = (real*) malloc(num_bytes);

  // Velocity fields
  waves->vz = (real*) malloc(num_bytes);
  waves->vx = (real*) malloc(num_bytes);
  waves->vy = (real*) malloc(num_bytes);
  waves->del1 = (real*) malloc(num_bytes);
  waves->del2 = (real*) malloc(num_bytes);
  waves->del3 = (real*) malloc(num_bytes);

  // Reset stress fields
  zero_data(waves->sxx, waves->nx_ghost, waves->ny_ghost, waves->nz_ghost);
  zero_data(waves->syy, waves->nx_ghost, waves->ny_ghost, waves->nz_ghost);
  zero_data(waves->szz, waves->nx_ghost, waves->ny_ghost, waves->nz_ghost);
  zero_data(waves->sxy, waves->nx_ghost, waves->ny_ghost, waves->nz_ghost);
  zero_data(waves->syz, waves->nx_ghost, waves->ny_ghost, waves->nz_ghost);
  zero_data(waves->sxz, waves->nx_ghost, waves->ny_ghost, waves->nz_ghost);

  // Reset velocity fields
  zero_data(waves->vz, waves->nx_ghost, waves->ny_ghost, waves->nz_ghost);
  zero_data(waves->vx, waves->nx_ghost, waves->ny_ghost, waves->nz_ghost);
  zero_data(waves->vz, waves->nx_ghost, waves->ny_ghost, waves->nz_ghost);
  zero_data(waves->del1, waves->nx_ghost, waves->ny_ghost, waves->nz_ghost);
  zero_data(waves->del2, waves->nx_ghost, waves->ny_ghost, waves->nz_ghost);
  zero_data(waves->del3, waves->nx_ghost, waves->ny_ghost, waves->nz_ghost);

  return waves;
}

void free_wave_arrays(std::shared_ptr<fdm3d_t> waves) {

  free(waves->sxx);
  free(waves->syy);
  free(waves->szz);

  free(waves->sxy);
  free(waves->syz);
  free(waves->sxz);

  free(waves->vz);
  free(waves->vx);
  free(waves->vy);

  free(waves->del1);
  free(waves->del2);
  free(waves->del3);
}

