/* Original Author: Espen B. Raknes <espen.raknes@ntnu.no>
 * Modified by: Mohammed Sourouri <mohammed.sourouri@ntnu.no>
 * Date: September 15, 2016
 */

#include "source.h"

// Comment: Inserting a source into the model. The source is inserted into the normal stress wave fields.
void insert_stress_source(std::shared_ptr<fdm3d_t> waves,
                          real* source,
                          const int _x,
                          const int _y,
                          const int _z,
                          const int it) {

  int _idx = idx(waves->nx_ghost, waves->ny_ghost, _x, _y, _z);

  waves->szz[_idx] += source[it] * waves->dt;
  waves->sxx[_idx] += source[it] * waves->dt;
  waves->syy[_idx] += source[it] * waves->dt;
}

// Inserting a source into modeling. The source is inserted into the source wave fields without scaling.
void insert_force_source(std::shared_ptr<fdm3d_t> waves,
                         real* source,
                         std::shared_ptr<model3d_t> model,
                         const int it,
                         const int _x,
                         const int _y,
                         const int _z,
                         const int direction,
                         const int type) {

  int _idx = idx(waves->nx_ghost, waves->ny_ghost, _x, _y, _z);

  if (type == 1) {
    // MONOPOLE
    if (direction == 1) {
      waves->vx[_idx] += source[it] * waves->dt * (1.0 / model->rho[_idx]);
    } else if (direction == 2) {
      waves->vy[_idx] += source[it] * waves->dt * (1.0 / model->rho[_idx]);
    } else {
      waves->vz[_idx] += source[it] * waves->dt * (1.0 / model->rho[_idx]);
    }
  } else {
    // DIPOLE
    waves->vx[idx(waves->nx_ghost, waves->ny_ghost, _x + 1, _y, _z)] +=
        source[it] * waves->dt * (1.0 / model->rho[_idx]) * (1 / (2.0 * waves->dx));
    waves->vx[idx(waves->nx_ghost, waves->ny_ghost, _x - 1, _y, _z)] -=
        source[it] * waves->dt * (1.0 / model->rho[_idx]) * (1 / (2.0 * waves->dx));

    waves->vy[idx(waves->nx_ghost, waves->ny_ghost, _x, _y + 1, _z)] +=
        source[it] * waves->dt * (1.0 / model->rho[_idx]) * (1 / (2.0 * waves->dy));
    waves->vy[idx(waves->nx_ghost, waves->ny_ghost, _x, _y - 1, _z)] -=
        source[it] * waves->dt * (1.0 / model->rho[_idx]) * (1 / (2.0 * waves->dy));

    waves->vz[idx(waves->nx_ghost, waves->ny_ghost, _x, _y, _z + 1)] +=
        source[it] * waves->dt * (1.0 / model->rho[_idx]) * (1 / (2.0 * waves->dz));
    waves->vz[idx(waves->nx_ghost, waves->ny_ghost, _x, _y, _z - 1)] -=
        source[it] * waves->dt * (1.0 / model->rho[_idx]) * (1 / (2.0 * waves->dz));
  }
}
