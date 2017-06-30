/* Original Author: Espen B. Raknes <espen.raknes@ntnu.no>
 * Modified by: Mohammed Sourouri <mohammed.sourouri@ntnu.no>
 * Date: September 19, 2016
 */

#ifndef RECEIVER3D_H
#define RECEIVER3D_H

#include "fd3d.h"
#include <fstream>

struct receiver3d_s {
  int n;        // Size for array
  int nt;        // Number of time steps
  int rectype;        // Type of receivers
  real* p;            // Array for pressure receivers
  real* vx;        // Array for Vx receivers
  real* vz;        // Array for Vz receivers
  real* vy;        // Array for Vy receivers
  bool P, Vz, Vx, Vy;    // Booleans to control which field is defined
  int* x;        // x position fo receiver
  int* y;        // z position fo receiver
  int* z;        // y position fo receiver
};

typedef struct receiver3d_s receiver3d_t;

std::shared_ptr<receiver3d_t> receiver3d_setup(const int _n,
                                               const int _ny,
                                               const bool _P,
                                               const bool _Vx,
                                               const bool _Vy,
                                               const bool _Vz);
int determine_receiver_value(const int Nz);
void setup_receiver_for_verification(std::shared_ptr <receiver3d_t> receiver,
                                     const int Nz,
                                     const int x_source,
                                     const int y_source,
                                     const int z_source);
void setup32(std::shared_ptr <receiver3d_t> receiver, const int x_source, const int y_source, const int z_source);
void setup64(std::shared_ptr <receiver3d_t> receiver, const int x_source, const int y_source, const int z_source);
void setup128(std::shared_ptr <receiver3d_t> receiver, const int x_source, const int y_source, const int z_source);
void setup256(std::shared_ptr <receiver3d_t> receiver, const int x_source, const int y_source, const int z_source);
void setup512(std::shared_ptr <receiver3d_t> receiver, const int x_source, const int y_source, const int z_source);
void setup1024(std::shared_ptr <receiver3d_t> receiver, const int x_source, const int y_source, const int z_source);

void save_receivers(std::shared_ptr<receiver3d_t> rec, std::shared_ptr<fdm3d_t> waves, const int _it);
void write_receiver_file(std::shared_ptr<receiver3d_t> rec, std::shared_ptr<dims_t> dims);
void free_receiver_arrays(std::shared_ptr<receiver3d_t> rec);

#endif //FD3D_RECEIVER3D_H
