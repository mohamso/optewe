/* Original Author: Espen B. Raknes <espen.raknes@ntnu.no>
 * Modified by: Mohammed Sourouri <mohammed.sourouri@ntnu.no>
 * Date: September 10, 2016
 */

#ifndef MODEL3D_H
#define MODEL3D_H

#include "dims.h"
#include "mem_utils.h"

struct model3d_s {
  real* input;    // Temporary input array
  real* vp;    // Input Vp model
  real* vs;    // Input Vs model
  real* rho;    // Input Rho model
  real* lambda;// Input lambda model
  real* mu;    // Input mu model
  real* l;    // Input Lambda model (compliance version of lambda)
  real* m;    // Input Mu model (compliance version of mu)
  bool Vp, Vs, Rho, Lambda, Mu, L, M; // Booleans set to 1 if arrays are created.
};

typedef struct model3d_s model3d_t;

std::shared_ptr<model3d_t> model_setup(std::shared_ptr<dims_t> dims);
void free_model_arrays(std::shared_ptr<model3d_t> model);
void set_uniform_model(std::shared_ptr<model3d_t> model,
                       std::shared_ptr<dims_t> dims,
                       const real _rho,
                       const real _vp,
                       const real _vs);

#endif // MODEL3D_H
