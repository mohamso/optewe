/* Author: Mohammed Sourouri <mohammed.sourouri@ntnu.no>
 * Date: January 16, 2012
 * Updated: September 3, 2016
 */

#ifndef PRINT_H
#define PRINT_H

#include "common.h"
#include <iostream>
#include <iomanip>

void print_application_info(std::string app_name, const int source_type, const int Nx, const int Ny,
                            const int Nz, const int Nt);
void print_omp_info(const unsigned int num_threads);
void print_perf_summary(const double mlups, const double compute_timer);
void print_3D(const real* __restrict__ buffer, const int Nx, const int Ny, const int Nz);
void print_2D(const real* __restrict__ buffer, const int Nx, const int Ny);

#endif //PRINT_H
