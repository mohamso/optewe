/* Original Author: Espen B. Raknes <espen.raknes@ntnu.no>
 * Modified by: Mohammed Sourouri <mohammed.sourouri@ntnu.no>
 * Date: September 9, 2016
 */

#include "dims.h"

std::shared_ptr<dims_t> size_setup(const int nx,
                   const int ny,
                   const int nz,
                   const int nt,
                   const int ghost_border,
                   const real dz,
                   const real dx,
                   const real dy,
                   const real dt) {

  std::shared_ptr<dims_t> dim ((dims_t*)malloc(sizeof(dims_t)), free_ptr());

  dim->dt = dt;
  dim->dx = dx;
  dim->dy = dy;
  dim->dz = dz;

  dim->nt = nt;

  dim->nx = nx;
  dim->ny = ny;
  dim->nz = nz;

  dim->ghost_border = ghost_border;

  // Ghost borders
  dim->nz_ghost = nz + 2 * ghost_border; // If free surface use nz + ghost_border
  dim->nx_ghost = nx + 2 * ghost_border;
  dim->ny_ghost = ny + 2 * ghost_border;

  return dim;
}
