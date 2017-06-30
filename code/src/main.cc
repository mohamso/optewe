/* Original Author: Espen B. Raknes <espen.raknes@ntnu.no>
 * Modified by: Mohammed Sourouri <mohammed.sourouri@ntnu.no>
 * Date: September 27, 2016
 * Updated: January 27, 2017
*/

#include <cstdint>
#include <sstream>
#include <iostream>
#include <cmath>
#include <chrono>
#include <vector>

#include "common.h"
#include "fd3d.h"
#include "model3d.h"

#ifdef SAVE_RECEIVERS
#include "receiver3d.h"
#endif

#include "differentiators.h"
#include "step_forward.h"
#include "source.h"
#include "print.h"

#ifdef HDEEM
#include <hdeem_cxx.hpp>
#include "nemi.h"
#endif

#include <x86_dvfs.h>

#ifdef VTK
#include "vtk.h"
#endif

#include <omp.h>

int main(int argc, char** argv) {

#ifdef HDEEM
  std::unique_ptr <hdeem::connection> hdeem;
  hdeem.reset(new hdeem::connection());
  hdeem->start();
  std::vector<kernel> kernels;
#endif

  int Nx = 0;
  int Ny = 0;
  int Nz = 0;
  int Nt = 0;

#ifdef STATIC_DVFS
  uint64_t coref = 0;
  uint64_t ucoref = 0;
#endif

  int ghost_cells = 0;
  int source_type = 0;

  const real kDz = 10.0;
  const real kDx = 10.0;
  const real kDy = 10.0;
  const real kDt = 0.001;

  std::stringstream string_buffer;

  for (int i = 1; i < argc; i++) {
    string_buffer << argv[i] << " ";
  }

  string_buffer >> Nx;
  string_buffer >> Ny;
  string_buffer >> Nz;
  string_buffer >> Nt;
#ifdef STATIC_DVFS
  string_buffer >> coref;
  string_buffer >> ucoref;
#endif
  string_buffer >> source_type;

  // Allocate source buffer
  size_t source_num_bytes = sizeof(real) * Nt;
  real* source = (real*) malloc(source_num_bytes);

  // Setup of the wavefields
  std::shared_ptr <dims_t> dims = size_setup(Nx, Ny, Nz, Nt, ghost_cells, kDz, kDx, kDy, kDt);
  std::shared_ptr <fdm3d_t> waves = fdm3d_setup(dims);
  std::shared_ptr <model3d_t> model = model_setup(dims);

  // Setup for a uniform model (medium: solid)
  const real kRho = 1000.0;
  const real kVp = 2200.0;
  const real kVs = 1000.0;

  set_uniform_model(model, dims, kRho, kVp, kVs);

  // Create source
  const real kF0 = 5.0;
  const real kT0 = 0.3;
  const int x_source = waves->nx_ghost / 2;
  const int y_source = waves->ny_ghost / 2;
  const int z_source = waves->nz_ghost / 2;
  const int source_dir = 1; // Force in x-direction

  omp_set_dynamic(0);

#pragma omp parallel for
  for (int i = 0; i < Nt; i++) {
    real arg = kPI * kF0 * (kDt * i - kT0);
    arg = arg * arg;
    source[i] = 10e3f * (2.0f * arg - 1.0f) * std::exp(-arg);
  }

  // Receiver setup
#ifdef SAVE_RECEIVERS
  const bool vx = true;
  const bool vy = true;
  const bool vz = true;
  const bool P = false;
  int n = determine_receiver_value(Nz);
  std::shared_ptr <receiver3d_t> receiver = receiver3d_setup(n, Nt, P, vx, vy, vz);
  setup_receiver_for_verification(receiver, Nz, x_source, y_source, z_source);
#endif

  // Unpack values
  const int nz_ghost = waves->nz_ghost;
  const int nx_ghost = waves->nx_ghost;
  const int ny_ghost = waves->ny_ghost;
  const real dt = waves->dt;

  dvfs_init();

  x86_adapt_device_type core_type = X86_ADAPT_CPU;
  x86_adapt_device_type uncore_type = X86_ADAPT_DIE;

  int fd = x86_adapt_get_all_devices(core_type);

  int num_cores = x86_adapt_get_nr_avaible_devices(core_type);
  int numa_nodes = x86_adapt_get_nr_avaible_devices(uncore_type);

  int pstate_idx = get_target_pstate(core_type);
  int uncore_min_idx = get_uncore_min_ratio(uncore_type);
  int uncore_max_idx = get_uncore_max_ratio(uncore_type);

#ifdef STATIC_DVFS
  set_all_core_freq(core_type, fd, pstate_idx, coref);
  set_uncore_freq(uncore_type, numa_nodes, uncore_min_idx, uncore_max_idx, ucoref);
#endif

  // Timer setup
  auto timer_start = std::chrono::high_resolution_clock::now();

  // Time stepping
  for (int it = 0; it < Nt; it++) {

    // Insert source
    if (source_type == 1) {
      insert_stress_source(waves, source, x_source, y_source, z_source, it);
    } else if (source_type == 2) {
      insert_force_source(waves, source, model, it, x_source, y_source, z_source, source_dir, 1);
    } else {
      insert_force_source(waves, source, model, it, x_source, y_source, z_source, source_dir, 2);
    }

    // Save receivers
#ifdef SAVE_RECEIVERS
    save_receivers(receiver, waves, it);
#endif

    // Compute Vx

// dx_forward
#ifdef DXF_DVFS
    set_all_core_freq(core_type, fd, pstate_idx, DXF_CORE);
    set_uncore_freq(uncore_type, numa_nodes, uncore_min_idx, uncore_max_idx, DXF_UNCORE);
#endif
#ifdef DXF_HDEEM
    auto dxf_time_start = std::chrono::high_resolution_clock::now();
    auto dxf_timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());
#endif
    dx_forward(waves->del1, waves->sxx, nx_ghost, ny_ghost, nz_ghost, 1.0f / waves->dx);
#ifdef DXF_HDEEM
    auto dxf_time_end = std::chrono::high_resolution_clock::now();
    double dxf_tstart = (double)dxf_timestamp.count();
    double dxf_rtime = (dxf_time_end-dxf_time_start).count();
    kernels.push_back(kernel("dxf", it, dxf_tstart, dxf_rtime));
#endif

// dz_backward
#ifdef DZB_DVFS
    set_all_core_freq(core_type, fd, pstate_idx, DZB_CORE);
    set_uncore_freq(uncore_type, numa_nodes, uncore_min_idx, uncore_max_idx, DZB_UNCORE);
#endif
#ifdef DZB_HDEEM
    auto dzb_time_start = std::chrono::high_resolution_clock::now();
    auto dzb_timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());
#endif
    dz_backward(waves->del2, waves->sxz, nx_ghost, ny_ghost, nz_ghost, 1.0f / waves->dz);
#ifdef DZB_HDEEM
    auto dzb_time_end = std::chrono::high_resolution_clock::now();
    double dzb_tstart = (double)dzb_timestamp.count();
    double dzb_rtime = (dzb_time_end-dzb_time_start).count();
    kernels.push_back(kernel("dzb", it, dzb_tstart, dzb_rtime));
#endif


// dy_backward
#ifdef DYB_DVFS
    set_all_core_freq(core_type, fd, pstate_idx, DYB_CORE);
    set_uncore_freq(uncore_type, numa_nodes, uncore_min_idx, uncore_max_idx, DYB_UNCORE);
#endif
#ifdef DYB_HDEEM
    auto dyb_time_start = std::chrono::high_resolution_clock::now();
    auto dyb_timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());
#endif
    dy_backward(waves->del3, waves->sxy, nx_ghost, ny_ghost, nz_ghost, 1.0f / waves->dy);
#ifdef DYB_HDEEM
    auto dyb_time_end = std::chrono::high_resolution_clock::now();
    double dyb_tstart = (double)dyb_timestamp.count();
    double dyb_rtime = (dyb_time_end-dyb_time_start).count();
    kernels.push_back(kernel("dyb", it, dyb_tstart, dyb_rtime));
#endif


// Compute_vx
#ifdef CVX_DVFS
    set_all_core_freq(core_type, fd, pstate_idx, CVX_CORE);
    set_uncore_freq(uncore_type, numa_nodes, uncore_min_idx, uncore_max_idx, CVX_UNCORE);
#endif
#ifdef CVX_HDEEM
    auto cvx_time_start = std::chrono::high_resolution_clock::now();
    auto cvx_timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());
#endif
    compute_vx(waves->vx, model->rho, waves->del1, waves->del2, waves->del3, dt, nx_ghost, ny_ghost, nz_ghost);
#ifdef CVX_HDEEM
    auto cvx_time_end = std::chrono::high_resolution_clock::now();
    double cvx_tstart = (double)cvx_timestamp.count();
    double cvx_rtime = (cvx_time_end-cvx_time_start).count();
    kernels.push_back(kernel("cvx", it, cvx_tstart, cvx_rtime));
#endif


    // Compute Vy

// dy_foward
#ifdef DYF_DVFS
    set_all_core_freq(core_type, fd, pstate_idx, DYF_CORE);
    set_uncore_freq(uncore_type, numa_nodes, uncore_min_idx, uncore_max_idx, DYF_UNCORE);
#endif
#ifdef DYF_HDEEM
    auto dyf_time_start = std::chrono::high_resolution_clock::now();
    auto dyf_timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());
#endif
    dy_forward(waves->del1, waves->syy, nx_ghost, ny_ghost, nz_ghost, 1.0f / waves->dy);
#ifdef DYF_HDEEM
    auto dyf_time_end = std::chrono::high_resolution_clock::now();
    double dyf_tstart = (double)dyf_timestamp.count();
    double dyf_rtime = (dyf_time_end-dyf_time_start).count();
    kernels.push_back(kernel("dyf", it, dyf_tstart, dyf_rtime));
#endif

// dz_backward
#ifdef DZB2_DVFS
    set_all_core_freq(core_type, fd, pstate_idx, DZB2_CORE);
    set_uncore_freq(uncore_type, numa_nodes, uncore_min_idx, uncore_max_idx, DZB2_UNCORE);
#endif
#ifdef DZB2_HDEEM
    auto dzb2_time_start = std::chrono::high_resolution_clock::now();
    auto dzb2_timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());
#endif
    dz_backward(waves->del2, waves->syz, nx_ghost, ny_ghost, nz_ghost, 1.0f / waves->dz);
#ifdef DZB2_HDEEM
    auto dzb2_time_end = std::chrono::high_resolution_clock::now();
    double dzb2_tstart = (double)dzb2_timestamp.count();
    double dzb2_rtime = (dzb2_time_end-dzb2_time_start).count();
    kernels.push_back(kernel("dzb2", it, dzb2_tstart, dzb2_rtime));
#endif


// dx_backward
#ifdef DXB_DVFS
    set_all_core_freq(core_type, fd, pstate_idx, DXB_CORE);
    set_uncore_freq(uncore_type, numa_nodes, uncore_min_idx, uncore_max_idx, DXB_UNCORE);
#endif
#ifdef DXB_HDEEM
    auto dxb_time_start = std::chrono::high_resolution_clock::now();
    auto dxb_timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());
#endif
    dx_backward(waves->del3, waves->sxy, nx_ghost, ny_ghost, nz_ghost, 1.0f / waves->dx);
#ifdef DXB_HDEEM
    auto dxb_time_end = std::chrono::high_resolution_clock::now();
    double dxb_tstart = (double)dxb_timestamp.count();
    double dxb_rtime = (dxb_time_end-dxb_time_start).count();
    kernels.push_back(kernel("dxb", it, dxb_tstart, dxb_rtime));
#endif


// compute_vy
#ifdef CVY_DVFS
    set_all_core_freq(core_type, fd, pstate_idx, CVY_CORE);
    set_uncore_freq(uncore_type, numa_nodes, uncore_min_idx, uncore_max_idx, CVY_UNCORE);
#endif
#ifdef CVY_HDEEM
    auto cvy_time_start = std::chrono::high_resolution_clock::now();
    auto cvy_timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());
#endif
    compute_vy(waves->vy, model->rho, waves->del1, waves->del2, waves->del3, dt, nx_ghost, ny_ghost, nz_ghost);
#ifdef CVY_HDEEM
    auto cvy_time_end = std::chrono::high_resolution_clock::now();
    double cvy_tstart = (double)cvy_timestamp.count();
    double cvy_rtime = (cvy_time_end-cvy_time_start).count();
    kernels.push_back(kernel("cvy", it, cvy_tstart, cvy_rtime));
#endif


    // Compute Vz

// dz_forward
#ifdef DZF_DVFS
    set_all_core_freq(core_type, fd, pstate_idx, DZF_CORE);
    set_uncore_freq(uncore_type, numa_nodes, uncore_min_idx, uncore_max_idx, DZF_UNCORE);
#endif
#ifdef DZF_HDEEM
    auto dzf_time_start = std::chrono::high_resolution_clock::now();
    auto dzf_timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());
#endif
    dz_forward(waves->del1, waves->szz, nx_ghost, ny_ghost, nz_ghost, 1.0f / waves->dz);
#ifdef DZF_HDEEM
    auto dzf_time_end = std::chrono::high_resolution_clock::now();
    double dzf_tstart = (double)dzf_timestamp.count();
    double dzf_rtime = (dzf_time_end-dzf_time_start).count();
    kernels.push_back(kernel("dzf", it, dzf_tstart, dzf_rtime));
#endif


// dx_backward
#ifdef DXB2_DVFS
    set_all_core_freq(core_type, fd, pstate_idx, DXB2_CORE);
    set_uncore_freq(uncore_type, numa_nodes, uncore_min_idx, uncore_max_idx, DXB2_UNCORE);
#endif
#ifdef DXB2_HDEEM
    auto dxb2_time_start = std::chrono::high_resolution_clock::now();
    auto dxb2_timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());
#endif
    dx_backward(waves->del2, waves->sxz, nx_ghost, ny_ghost, nz_ghost, 1.0f / waves->dx);
#ifdef DXB2_HDEEM
    auto dxb2_time_end = std::chrono::high_resolution_clock::now();
    double dxb2_tstart = (double)dxb2_timestamp.count();
    double dxb2_rtime = (dxb2_time_end-dxb2_time_start).count();
    kernels.push_back(kernel("dxb2", it, dxb2_tstart, dxb2_rtime));
#endif


// dy_backward
#ifdef DYB2_DVFS
    set_all_core_freq(core_type, fd, pstate_idx, DYB2_CORE);
    set_uncore_freq(uncore_type, numa_nodes, uncore_min_idx, uncore_max_idx, DYB2_UNCORE);
#endif
#ifdef DYB2_HDEEM
    auto dyb2_time_start = std::chrono::high_resolution_clock::now();
    auto dyb2_timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());
#endif
    dy_backward(waves->del3, waves->syz, nx_ghost, ny_ghost, nz_ghost, 1.0f / waves->dy);
#ifdef DYB2_HDEEM
    auto dyb2_time_end = std::chrono::high_resolution_clock::now();
    double dyb2_tstart = (double)dyb2_timestamp.count();
    double dyb2_rtime = (dyb2_time_end-dyb2_time_start).count();
    kernels.push_back(kernel("dyb2", it, dyb2_tstart, dyb2_rtime));
#endif


// compute_vz
#ifdef CVZ_DVFS
    set_all_core_freq(core_type, fd, pstate_idx, CVZ_CORE);
    set_uncore_freq(uncore_type, numa_nodes, uncore_min_idx, uncore_max_idx, CVZ_UNCORE);
#endif
#ifdef CVZ_HDEEM
    auto cvz_time_start = std::chrono::high_resolution_clock::now();
    auto cvz_timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());
#endif
    compute_vz(waves->vz, model->rho, waves->del1, waves->del2, waves->del3, dt, nx_ghost, ny_ghost, nz_ghost);
#ifdef CVZ_HDEEM
    auto cvz_time_end = std::chrono::high_resolution_clock::now();
    double cvz_tstart = (double)cvz_timestamp.count();
    double cvz_rtime = (cvz_time_end-cvz_time_start).count();
    kernels.push_back(kernel("cvz", it, cvz_tstart, cvz_rtime));
#endif


    // Compute Sxx, Syy, Szz

// dz_backward
#ifdef DZB3_DVFS
    set_all_core_freq(core_type, fd, pstate_idx, DZB3_CORE);
    set_uncore_freq(uncore_type, numa_nodes, uncore_min_idx, uncore_max_idx, DZB3_UNCORE);
#endif
#ifdef DZB3_HDEEM
    auto dzb3_time_start = std::chrono::high_resolution_clock::now();
    auto dzb3_timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());
#endif
    dz_backward(waves->del1, waves->vz, nx_ghost, ny_ghost, nz_ghost, 1.0f / waves->dz);
#ifdef DZB3_HDEEM
    auto dzb3_time_end = std::chrono::high_resolution_clock::now();
    double dzb3_tstart = (double)dzb3_timestamp.count();
    double dzb3_rtime = (dzb3_time_end-dzb3_time_start).count();
    kernels.push_back(kernel("dzb3", it, dzb3_tstart, dzb3_rtime));
#endif


// dx_backward
#ifdef DXB3_DVFS
    set_all_core_freq(core_type, fd, pstate_idx, DXB3_CORE);
    set_uncore_freq(uncore_type, numa_nodes, uncore_min_idx, uncore_max_idx, DXB3_UNCORE);
#endif
#ifdef DXB3_HDEEM
    auto dxb3_time_start = std::chrono::high_resolution_clock::now();
    auto dxb3_timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());
#endif
    dx_backward(waves->del2, waves->vx, nx_ghost, ny_ghost, nz_ghost, 1.0f / waves->dx);
#ifdef DXB3_HDEEM
    auto dxb3_time_end = std::chrono::high_resolution_clock::now();
    double dxb3_tstart = (double)dxb3_timestamp.count();
    double dxb3_rtime = (dxb3_time_end-dxb3_time_start).count();
    kernels.push_back(kernel("dxb3", it, dxb3_tstart, dxb3_rtime));
#endif


// dy_backward
#ifdef DYB3_DVFS
    set_all_core_freq(core_type, fd, pstate_idx, DYB3_CORE);
    set_uncore_freq(uncore_type, numa_nodes, uncore_min_idx, uncore_max_idx, DYB3_UNCORE);
#endif
#ifdef DYB3_HDEEM
    auto dyb3_time_start = std::chrono::high_resolution_clock::now();
    auto dyb3_timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());
#endif
    dy_backward(waves->del3, waves->vy, nx_ghost, ny_ghost, nz_ghost, 1.0f / waves->dy);
#ifdef DYB3_HDEEM
    auto dyb3_time_end = std::chrono::high_resolution_clock::now();
    double dyb3_tstart = (double)dyb3_timestamp.count();
    double dyb3_rtime = (dyb3_time_end-dyb3_time_start).count();
    kernels.push_back(kernel("dyb3", it, dyb3_tstart, dyb3_rtime));
#endif


// compute_sxx_syy_szz
#ifdef CSXXSYYSZZ_DVFS
    set_all_core_freq(core_type, fd, pstate_idx, CSXXSYYSZZ_CORE);
    set_uncore_freq(uncore_type, numa_nodes, uncore_min_idx, uncore_max_idx, CSXXSYYSZZ_UNCORE);
#endif
#ifdef CSXXSYYSZZ_HDEEM
    auto csxxsyyszz_time_start = std::chrono::high_resolution_clock::now();
    auto csxxsyyszz_timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());
#endif
    compute_sxx_syy_szz(waves->sxx, waves->syy, waves->szz, waves->del1, waves->del2, waves->del3,
                        model->lambda, model->mu, dt, nx_ghost, ny_ghost, nz_ghost);
#ifdef CSXXSYYSZZ_HDEEM
    auto csxxsyyszz_time_end = std::chrono::high_resolution_clock::now();
    double csxxsyyszz_tstart = (double)csxxsyyszz_timestamp.count();
    double csxxsyyszz_rtime = (csxxsyyszz_time_end-csxxsyyszz_time_start).count();
    kernels.push_back(kernel("csxxsyyszz", it, csxxsyyszz_tstart, csxxsyyszz_rtime));
#endif


    // Compute Sxy

// dy_forward
#ifdef DYF2_DVFS
    set_all_core_freq(core_type, fd, pstate_idx, DYF2_CORE);
    set_uncore_freq(uncore_type, numa_nodes, uncore_min_idx, uncore_max_idx, DYF2_UNCORE);
#endif
#ifdef DYF2_HDEEM
    auto dyf2_time_start = std::chrono::high_resolution_clock::now();
    auto dyf2_timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());
#endif
    dy_forward(waves->del1, waves->vx, nx_ghost, ny_ghost, nz_ghost, 1.0f / waves->dy);
#ifdef DYF2_HDEEM
    auto dyf2_time_end = std::chrono::high_resolution_clock::now();
    double dyf2_tstart = (double)dyf2_timestamp.count();
    double dyf2_rtime = (dyf2_time_end-dyf2_time_start).count();
    kernels.push_back(kernel("dyf2", it, dyf2_tstart, dyf2_rtime));
#endif


// dx_forward
#ifdef DXF2_DVFS
    set_all_core_freq(core_type, fd, pstate_idx, DXF2_CORE);
    set_uncore_freq(uncore_type, numa_nodes, uncore_min_idx, uncore_max_idx, DXF2_UNCORE);
#endif
#ifdef DXF2_HDEEM
    auto dxf2_time_start = std::chrono::high_resolution_clock::now();
    auto dxf2_timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());
#endif
    dx_forward(waves->del2, waves->vy, nx_ghost, ny_ghost, nz_ghost, 1.0f / waves->dx);
#ifdef DXF2_HDEEM
    auto dxf2_time_end = std::chrono::high_resolution_clock::now();
    double dxf2_tstart = (double)dxf2_timestamp.count();
    double dxf2_rtime = (dxf2_time_end-dxf2_time_start).count();
    kernels.push_back(kernel("dxf2", it, dxf2_tstart, dxf2_rtime));
#endif


// compute_sxy
#ifdef CSXY_DVFS
    set_all_core_freq(core_type, fd, pstate_idx, CSXY_CORE);
    set_uncore_freq(uncore_type, numa_nodes, uncore_min_idx, uncore_max_idx, CSXY_UNCORE);
#endif
#ifdef CSXY_HDEEM
    auto csxy_time_start = std::chrono::high_resolution_clock::now();
    auto csxy_timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());
#endif
    compute_sxy(waves->sxy, model->mu, waves->del1, waves->del2, dt, nx_ghost, ny_ghost, nz_ghost);
#ifdef CSXY_HDEEM
    auto csxy_time_end = std::chrono::high_resolution_clock::now();
    double csxy_tstart = (double)csxy_timestamp.count();
    double csxy_rtime = (csxy_time_end-csxy_time_start).count();
    kernels.push_back(kernel("csxy", it, csxy_tstart, csxy_rtime));
#endif


    // Compute Syz

// dz_forward
#ifdef DZF2_DVFS
    set_all_core_freq(core_type, fd, pstate_idx, DZF2_CORE);
    set_uncore_freq(uncore_type, numa_nodes, uncore_min_idx, uncore_max_idx, DZF2_UNCORE);
#endif
#ifdef DZF2_HDEEM
    auto dzf2_time_start = std::chrono::high_resolution_clock::now();
    auto dzf2_timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());
#endif
    dz_forward(waves->del1, waves->vy, nx_ghost, ny_ghost, nz_ghost, 1.0f / waves->dz);
#ifdef DZF2_HDEEM
    auto dzf2_time_end = std::chrono::high_resolution_clock::now();
    double dzf2_tstart = (double)dzf2_timestamp.count();
    double dzf2_rtime = (dzf2_time_end-dzf2_time_start).count();
    kernels.push_back(kernel("dzf2", it, dzf2_tstart, dzf2_rtime));
#endif


// dy_forward
#ifdef DYF3_DVFS
    set_all_core_freq(core_type, fd, pstate_idx, DYF3_CORE);
    set_uncore_freq(uncore_type, numa_nodes, uncore_min_idx, uncore_max_idx, DYF3_UNCORE);
#endif
#ifdef DYF3_HDEEM
    auto dyf3_time_start = std::chrono::high_resolution_clock::now();
    auto dyf3_timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());
#endif
    dy_forward(waves->del2, waves->vz, nx_ghost, ny_ghost, nz_ghost, 1.0f / waves->dy);
#ifdef DYF3_HDEEM
    auto dyf3_time_end = std::chrono::high_resolution_clock::now();
    double dyf3_tstart = (double)dyf3_timestamp.count();
    double dyf3_rtime = (dyf3_time_end-dyf3_time_start).count();
    kernels.push_back(kernel("dyf3", it, dyf3_tstart, dyf3_rtime));
#endif


// compute_syz
#ifdef CSYZ_DVFS
    set_all_core_freq(core_type, fd, pstate_idx, CSYZ_CORE);
    set_uncore_freq(uncore_type, numa_nodes, uncore_min_idx, uncore_max_idx, CSYZ_UNCORE);
#endif
#ifdef CSYZ_HDEEM
    auto csyz_time_start = std::chrono::high_resolution_clock::now();
    auto csyz_timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());
#endif
    compute_syz(waves->syz, model->mu, waves->del1, waves->del2, dt, nx_ghost, ny_ghost, nz_ghost);
#ifdef CSYZ_HDEEM
    auto csyz_time_end = std::chrono::high_resolution_clock::now();
    double csyz_tstart = (double)csyz_timestamp.count();
    double csyz_rtime = (csyz_time_end-csyz_time_start).count();
    kernels.push_back(kernel("csyz", it, csyz_tstart, csyz_rtime));
#endif


    // Compute Sxz

// dx_forward
#ifdef DXF3_DVFS
    set_all_core_freq(core_type, fd, pstate_idx, DXF3_CORE);
    set_uncore_freq(uncore_type, numa_nodes, uncore_min_idx, uncore_max_idx, DXF3_UNCORE);
#endif
#ifdef DXF3_HDEEM
    auto dxf3_time_start = std::chrono::high_resolution_clock::now();
    auto dxf3_timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());
#endif
    dx_forward(waves->del1, waves->vz, nx_ghost, ny_ghost, nz_ghost, 1.0f / waves->dx);
#ifdef DXF3_HDEEM
    auto dxf3_time_end = std::chrono::high_resolution_clock::now();
    double dxf3_tstart = (double)dxf3_timestamp.count();
    double dxf3_rtime = (dxf3_time_end-dxf3_time_start).count();
    kernels.push_back(kernel("dxf3", it, dxf3_tstart, dxf3_rtime));
#endif


// dz_forward
#ifdef DZF3_DVFS
    set_all_core_freq(core_type, fd, pstate_idx, DZF3_CORE);
    set_uncore_freq(uncore_type, numa_nodes, uncore_min_idx, uncore_max_idx, DZF3_UNCORE);
#endif
#ifdef DZF3_HDEEM
    auto dzf3_time_start = std::chrono::high_resolution_clock::now();
    auto dzf3_timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());
#endif
    dz_forward(waves->del2, waves->vx, nx_ghost, ny_ghost, nz_ghost, 1.0f / waves->dz);
#ifdef DZF3_HDEEM
    auto dzf3_time_end = std::chrono::high_resolution_clock::now();
    double dzf3_tstart = (double)dzf3_timestamp.count();
    double dzf3_rtime = (dzf3_time_end-dzf3_time_start).count();
    kernels.push_back(kernel("dzf3", it, dzf3_tstart, dzf3_rtime));
#endif


// compute_sxz
#ifdef CSXZ_DVFS
    set_all_core_freq(core_type, fd, pstate_idx, CSXZ_CORE);
    set_uncore_freq(uncore_type, numa_nodes, uncore_min_idx, uncore_max_idx, CSXZ_UNCORE);
#endif
#ifdef CSXZ_HDEEM
    auto csxz_time_start = std::chrono::high_resolution_clock::now();
    auto csxz_timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());
#endif
    compute_sxz(waves->sxz, model->mu, waves->del1, waves->del2, dt, nx_ghost, ny_ghost, nz_ghost);
#ifdef CSXZ_HDEEM
    auto csxz_time_end = std::chrono::high_resolution_clock::now();
    double csxz_tstart = (double)csxz_timestamp.count();
    double csxz_rtime = (csxz_time_end-csxz_time_start).count();
    kernels.push_back(kernel("csxz", it, csxz_tstart, csxz_rtime));
#endif

    // Write out snapshot of wave fields
#ifdef VTK
    if (it%100 == 0) {
      std::stringstream filename_sstream;
      filename_sstream << "waves_test_" << it;
      std::string filename;
      filename_sstream >> filename;
      export_to_vtk(waves, filename);
    }
#endif

  }

  auto timer_stop = std::chrono::high_resolution_clock::now();
  double elapsed_seconds = (timer_stop - timer_start).count() * 1e-9f;

#ifdef HDEEM
  hdeem->stop();
  auto stats = hdeem->get_stats();
  process_kernel_energy(*hdeem, kernels);
#endif

  dvfs_finalize();

  // Write to receiver file
#ifdef SAVE_RECEIVERS
  write_receiver_file(receiver, dims);
#endif

  // Print app statistics
  double mlups = (double)(Nt)*((Nx * Ny * Nz) * 1e-6f) / elapsed_seconds;
  print_application_info("OptEWE [OpenMP]", source_type, Nx, Ny, Nz, Nt);
  print_perf_summary(mlups, elapsed_seconds);

#pragma omp parallel
  {
    unsigned int num_threads = omp_get_num_threads();
#pragma omp single
    print_omp_info(num_threads);
  }

  // Clear memory
  free(source);
  free_wave_arrays(waves);
  free_model_arrays(model);

#ifdef SAVE_RECEIVERS
  free_receiver_arrays(receiver);
#endif

  return 0;
}
