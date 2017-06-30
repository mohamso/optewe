/* Original Author: Espen B. Raknes <espen.raknes@ntnu.no>
 * Modified by: Mohammed Sourouri <mohammed.sourouri@ntnu.no>
 * Date: September 9, 2016
 */

#ifndef DIMS_H
#define DIMS_H

#include <cstdlib>
#include <cstring>
#include <memory>
#include <functional>
#include "common.h"

struct dims_s {
  int nz; // Size for z-axis (dimension 1)
  int nx; // Size for x-axis (dimension 2)
  int ny; // Size for y-axis (dimension 3)
  int nt; // Size for time axis
  int ghost_border;    // Number of points in ghost border layer for PML layers
  int nz_ghost;    // Size for z-axis (dimension 1) with ghost borders included
  int nx_ghost;    // Size for x-axis (dimension 2) with ghost borders included
  int ny_ghost;    // Size for y-axis (dimension 3) with ghost borders included
  real dz;    // Sampling for z-axis (dimension 1)
  real dx;    // Sampling for x-axis (dimension 2)
  real dy;    // Sampling for y-axis (dimension 3)
  real dt;    // Sampling for time axis
};

typedef struct dims_s dims_t;

struct free_ptr {
  void operator()(void* x) { free(x); }
};

std::shared_ptr<dims_t> size_setup(const int nx,
                                   const int ny,
                                   const int nz,
                                   const int nt,
                                   const int ghost_border,
                                   const real dz,
                                   const real dx,
                                   const real dy,
                                   const real dt);

#endif // DIMS_H
