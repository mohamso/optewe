/* Original Author: Espen B. Raknes <espen.raknes@ntnu.no>
 * Modified by: Mohammed Sourouri <mohammed.sourouri@ntnu.no>
 * Date: September 10, 2016
 */

#include "model3d.h"

// Initialization of the model structure
std::shared_ptr<model3d_t> model_setup(std::shared_ptr<dims_t> dims) {

  // Create wave structure
  std::shared_ptr<model3d_t> model((model3d_t*) malloc(sizeof(model3d_t)), free_ptr());

  // Allocate input arrays
  size_t num_bytes = sizeof(real) * ((dims->nx) * (dims->ny) * (dims->nz));
  size_t num_bytes_ghost = sizeof(real) * ((dims->nx_ghost)) * ((dims->ny_ghost)) * ((dims->nz_ghost));

  model->input = (real*) malloc(num_bytes);
  model->rho = (real*) malloc(num_bytes_ghost);
  model->lambda = (real*) malloc(num_bytes_ghost);
  model->mu = (real*) malloc(num_bytes_ghost);

  // Reset input arrays
  zero_data(model->input, dims->nx, dims->ny, dims->nz);
  zero_data(model->rho, dims->nx_ghost, dims->ny_ghost, dims->nz_ghost);
  zero_data(model->lambda, dims->nx_ghost, dims->ny_ghost, dims->nz_ghost);
  zero_data(model->mu, dims->nx_ghost, dims->ny_ghost, dims->nz_ghost);

  // Assign bool values
  model->Vp = 0;
  model->Vs = 0;
  model->Rho = 1;
  model->Lambda = 1;
  model->Mu = 1;
  model->L = 0;
  model->M = 0;

  return model;
}

void set_uniform_model(std::shared_ptr<model3d_t> model,
                       std::shared_ptr<dims_t> dims,
                       const real _rho,
                       const real _vp,
                       const real _vs) {
  int size = (dims->nx_ghost) * (dims->ny_ghost) * (dims->nz_ghost);

  // Compute lambda and mu
  real mu = _rho * _vs * _vs;
  real lambda = _vp * _vp * _rho - 2 * mu;

  for (int i = 0; i < size; i++) {
    model->rho[i] = _rho;
    model->lambda[i] = lambda;
    model->mu[i] = mu;
  }
}

void free_model_arrays(std::shared_ptr<model3d_t> model) {
  free(model->input);
  free(model->rho);
  free(model->lambda);
  free(model->mu);
}
