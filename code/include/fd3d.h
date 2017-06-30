/* Original Author: Espen B. Raknes <espen.raknes@ntnu.no>
 * Modified by: Mohammed Sourouri <mohammed.sourouri@ntnu.no>
 * Date: September 10, 2016
 */

#ifndef FD3D_H
#define FD3D_H

#include "dims.h"
#include "mem_utils.h"

struct fdm3d_s {
  real* szz;    // Temporary Szz stress field for storage in one time step
  real* sxx;    // Temporary Sxx stress field for storage in one time step
  real* syy;    // Temporary Syy stress field for storage in one time step
  real* sxy;    // Temporary Sxy stress field for storage in one time step
  real* syz;    // Temporary Syz stress field for storage in one time step
  real* sxz;    // Temporary Sxz stress field for storage in one time step
  real* vz;    // Temporary Vz velocity field for storage in one time step
  real* vx;    // Temporary Vx velocity field for storage in one time step
  real* vy;    // Temporary Vy velocity field
  real* del1;    // Temporary differentiator field
  real* del2;    // Temporary differentiator field
  real* del3;    // Temporary differentiator field
  bool free_surface;    // If we have free surface or not
  int ghost_border;    // Number of points in ghost border for PML layers
  int nt;        // Size for time axis
  int nz;        // Size for z-axis (dimension 1)
  int nx;        // Size for x-axis (dimension 2)
  int ny;        // Size for y-axis (dimension 3)
  real dt;    // Sampling for time axis
  real dz;    // Sampling for z-axis (dimension 1)
  real dx;    // Sampling for x-axis (dimension 2)
  real dy;    // Sampling for y-axis (dimension 3)
  int nz_ghost;    // Size for z-axis (dimension 1) with ghost borders included
  int nx_ghost;    // Size for x-axis (dimension 2) with ghost borders included
  int ny_ghost;    // Size for y-axis (dimension 3) with ghost borders included
};

typedef struct fdm3d_s fdm3d_t;

std::shared_ptr<fdm3d_t> fdm3d_setup(std::shared_ptr<dims_t> dims);
void free_wave_arrays(std::shared_ptr<fdm3d_t> waves);

#endif // FD3D_H
